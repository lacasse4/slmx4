/*
 * showfreq2.cpp
 * Second attempt to find frequency out of radar scan
 *   - Compute difference between subsequent scans
 *   - Filter the difference in the time domain
 *   - Convolution in the spatial domain with derivative masks
 *   - Add the absolute values of the plus and minus derivative
 * 
 * This path was abandoned.  Not successful.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "slmx4_vcom.h"
#include "serialib.h"
#include "frameFilter.h"
#include "rms.h"
#include "zeroxing.h"

#define MAX_FRAME_SIZE 188
#define NUM_FRAMES 200
#define FILTER_SIZE 9
#define RMS_THRESHOLD 0.005    // experimentaly set with "DATA-iter64-pps128-rms-141ms"

#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

// derivative masks
// given x + x/2 + x/4 + x/16 = 1.0, solution is: x = 0.5517
float mask_plus[FILTER_SIZE]  = {  0.0345,  0.1379,  0.2759,  0.5517, 0.0, -0.5517, -0.2759, -0.1379, -0.0345 };
float mask_minus[FILTER_SIZE] = { -0.0345, -0.1379, -0.2759, -0.5517, 0.0,  0.5517,  0.2759,  0.1379,  0.0345 };

slmx4 sensor;
timeOut timer;
float acquisition_frame[MAX_FRAME_SIZE*2];
float raw_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE];
// float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE];
// float max_frames[NUM_FRAMES][MAX_FRAME_SIZE];
// float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float plus_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float minus_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float add_abs_frames[NUM_FRAMES][MAX_FRAME_SIZE];

int   max_indices[NUM_FRAMES];
float frequencies[NUM_FRAMES];
int   valid_freq[NUM_FRAMES];
unsigned long int times[NUM_FRAMES];

frameFilter _filters_mem[MAX_FRAME_SIZE];
frameFilter* filters[MAX_FRAME_SIZE];

rms_t _rms_mem[MAX_FRAME_SIZE];
rms_t* rms[MAX_FRAME_SIZE];

zeroxing_t _zeroxing_mem[MAX_FRAME_SIZE];
zeroxing_t* zeroxing[MAX_FRAME_SIZE];


void  difference(float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples);
void  apply_frame_filters(float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, frameFilter** filters);
void  apply_rms_filters(float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, rms_t** rms);
void  find_max(float max_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int max_indices[NUM_FRAMES]);
void  compute_zeroxing(float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, float sampling_rate, zeroxing_t** zeroxing);
void  find_frequency(float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int max_indices[NUM_FRAMES], float frequencies[NUM_FRAMES], int valid_freq[NUM_FRAMES]);
void  convolution(float out_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, float mask[FILTER_SIZE]);
void  add_absolute(float out_frames[NUM_FRAMES][MAX_FRAME_SIZE], float a_frames[NUM_FRAMES][MAX_FRAME_SIZE], float b_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples);


void  display_slmx4_status();
void  clean_up(int sig);
void  launch_viewer();
int   write_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE], slmx4* sensor);


int main(int argc, char* argv[])
{
	// unlink(DATA_FILE_NAME);
	// if (argc == 1) {
	// 	mkfifo(DATA_FILE_NAME, FIFO_MODE);
	// 	launch_viewer();
	// }

	// Execute clean_up on CTRL_C
	signal(SIGINT, clean_up);

	// Initialise static memory
	memset(acquisition_frame, 0, MAX_FRAME_SIZE*2*sizeof(float));
	memset(raw_frames,        0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(diff_frames,       0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(filtered_frames,   0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	// memset(rms_frames,        0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	// memset(max_frames,        0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	// memset(freq_frames,       0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(plus_frames,        0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(minus_frames,       0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(add_abs_frames,     0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));


	memset(max_indices,       0, NUM_FRAMES*sizeof(int));
	memset(frequencies,       0, NUM_FRAMES*sizeof(float));
	memset(valid_freq,        0, NUM_FRAMES*sizeof(int));

	// Use static memory for the various filter. Initialise filter buffers
	for (int i = 0; i < MAX_FRAME_SIZE; i++) {
		filters[i] = &_filters_mem[i];
		frameFilter_init(filters[i]);
	}

	for (int i = 0; i < MAX_FRAME_SIZE; i++) {
		rms[i] = &_rms_mem[i];
		rms_init(rms[i]);
	}

   	for (int i = 0; i < MAX_FRAME_SIZE; i++) {
		zeroxing[i] = &_zeroxing_mem[i];
		zeroxing_init(zeroxing[i]);
	}

	// Check if the SLMX4 sensor is connected and available
	printf("Checking serial port: %s\n", SERIAL_PORT);
	if (access(SERIAL_PORT, F_OK) < 0) {
		fprintf(stderr, "ERROR: serial port NOT available: %s\n", SERIAL_PORT);		
		exit(EXIT_FAILURE);
	}

	// Initialise sensor
	printf("Initialize SLMX4 sensor\n");
	if (sensor.begin(SERIAL_PORT) == EXIT_FAILURE) {
		exit(EXIT_FAILURE);
	}

	// Set sensor operating mode, that is:
	//   - 64 iterations
	//   - 128 pulses per step (pps)
	//   - dac minimum value 896   (1024 - 128)
	//   - dac maximum value 1152  (1024 + 128)
	//   - downconversion and decimation enabled (ddc = 1)
	// The 4 first values were experimentaly set.
	// The iteration and pps values yield a smooth signal (less noise)
	// but increase the data acquisition time between frames.
	// By increasing dac_min and reducing dac_max, the data acquisition
	// time is reduced. This application does not require a larger range.
	// With this setup, the acquition rate is about 1 frame each 141.7 ms,
	// or 7.05 frames per second.
	//
	// Setting ddc to 1 enable the downconverion and decimation algorithm 
	// available on the SLMX4 sensor. It demodulates the radar signal 
	// from its carrier frequency. See:
	// https://sensorlogicinc.github.io/modules/docs/XTAN-13_XeThruX4RadarUserGuide_rev_a.pdf section 5.
	// The result is 188 points in complex numbers (float*2). 
	// Each complex data points is rapidely converted to there modulus 
	// in get_frame_normalized().   

    sensor.set_value_by_name("VarSetValue_ByName(iterations,64)");
    sensor.set_value_by_name("VarSetValue_ByName(pps,128)");
    // sensor.set_value_by_name("VarSetValue_ByName(fs,2.9)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_min,896)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_max,1152)");
    // sensor.set_value_by_name("VarSetValue_ByName(frame_start,2.0)");
    // sensor.set_value_by_name("VarSetValue_ByName(frame_end,4.0)");
    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");
	display_slmx4_status();

	// Acquire data
	printf("Data aquisition starts\n");
    // float sampling_frequency = 0.0;
	for (int i = 0; i < NUM_FRAMES; i++) {
    	timer.initTimer();

		sensor.get_frame_normalized(acquisition_frame, POWER_IN_WATT);
		memcpy(raw_frames[i], acquisition_frame, sensor.get_num_samples()*sizeof(float));

		times[i] = timer.elapsedTime_ms();
        // sampling_frequency = 1000.0 / times[i];
	}

	// Process data
	printf("Data processing starts\n");
	difference(diff_frames, raw_frames, sensor.get_num_samples());
	apply_frame_filters(filtered_frames, diff_frames, sensor.get_num_samples(), filters);
    convolution(plus_frames, filtered_frames, sensor.get_num_samples(), mask_plus);
    convolution(minus_frames, filtered_frames, sensor.get_num_samples(), mask_minus);
    add_absolute(add_abs_frames, plus_frames, minus_frames, sensor.get_num_samples());

    // apply_rms_filters(rms_frames, filtered_frames, sensor.get_num_samples(), rms); 
    // compute_zeroxing(freq_frames, filtered_frames, rms_frames, sensor.get_num_samples(), sampling_frequency, zeroxing);
    // find_max(max_frames, rms_frames, sensor.get_num_samples(), max_indices);
    // find_frequency(freq_frames, sensor.get_num_samples(), max_indices, frequencies, valid_freq);

	// for (int i; i < NUM_FRAMES; i++) {
	// 	printf("%4d, %4.1f rpm\n", valid_freq[i], frequencies[i]*60.0);
	// }

	// Write results to disk
	printf("Write files to disk\n");
	write_file("RAW_FRAMES", raw_frames, &sensor);
	write_file("DIFF_FRAMES", diff_frames, &sensor);
	write_file("FILTERED_FRAMES", filtered_frames, &sensor);
	// write_file("RMS_FRAMES", rms_frames, &sensor);
	// write_file("MAX_FRAMES", max_frames, &sensor);
	// write_file("FREQ_FRAMES", freq_frames, &sensor);
	write_file("PLUS_FRAMES", plus_frames, &sensor);
	write_file("MINUS_FRAMES", minus_frames, &sensor);
	write_file("ADD_ABS_FRAMES", add_abs_frames, &sensor);

	float average = 0.0;
	for (int i = 0; i < NUM_FRAMES; i++) {
		average += times[i];
	}
	printf("Average acquisiton time = %f ms\n", average/NUM_FRAMES);

	clean_up(0);

	return EXIT_SUCCESS;
}

// Write of text file containing NUM_FRAMES frames
int write_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE], slmx4* sensor)
{
	// Write data to file
	FILE* fd = fopen(filename, "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open %s for writing\n", filename);
		return 1;
	}	

	for (int i = 0; i < NUM_FRAMES; i++) {
		fprintf(fd, "%d\n", sensor->get_num_samples());
		fprintf(fd, "%f\n", sensor->get_frame_start());
		fprintf(fd, "%f\n", sensor->get_frame_end());
		for (int j = 0; j < sensor->get_num_samples(); j++) {
			fprintf(fd,"%f\n", frames[i][j]);
		}	
	}

	fclose(fd);
	return 0;
}

// Ouput slmx4 status to console
void display_slmx4_status() 
{
	printf("dac_min           = %d\n", sensor.get_dac_min());
	printf("dac_max           = %d\n", sensor.get_dac_max());
	printf("dac_step          = %d\n", sensor.get_dac_step());
	printf("pps               = %d\n", sensor.get_pps());
	printf("iterations        = %d\n", sensor.get_iterations());
	printf("prf_div           = %d\n", sensor.get_prf_div());
	printf("prf               = %e\n", sensor.get_prf());
	printf("fs                = %e\n", sensor.get_fs());
	printf("num_samples       = %d\n", sensor.get_num_samples());
	printf("frame_length      = %d\n", sensor.get_frame_length());
	printf("rx_wait           = %d\n", sensor.get_rx_wait());
	printf("tx_region         = %d\n", sensor.get_tx_region());
	printf("tx_power          = %d\n", sensor.get_tx_power());
	printf("ddc_en            = %d\n", sensor.is_ddc_en());
	printf("frame_offset      = %e\n", sensor.get_frame_offset());
	printf("frame_start       = %e\n", sensor.get_frame_start());
	printf("frame_end         = %e\n", sensor.get_frame_end());
	printf("sweep_time        = %e\n", sensor.get_sweep_time());
	printf("unambiguous_range = %e\n", sensor.get_unambiguous_range());
	printf("res               = %e\n", sensor.get_res());
	printf("fs_rf             = %e\n", sensor.get_fs_rf());
}

// Must be executed before exit in order to leave the sensor in a stable mode
void clean_up(int sig)
{
	printf("\nClose sensor\n");
	sensor.end();
	// unlink(DATA_FILE_NAME);
	exit(EXIT_SUCCESS);
}

// Lauch a viewer in a separate process
// Inter process communication is done by FIFO.
void launch_viewer()
{
	int pid = fork();
	if (pid == 0) {
		execlp("python", "python", "python/plotdiff.py", NULL);
		fprintf(stderr, "ERROR - Unable to start viewer\n");
		exit(EXIT_FAILURE);
	}
}

// Compute the difference between 2 frames
void  difference(float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples)
{
	for (int i = 1; i < NUM_FRAMES; i++) {
		for (int j = 0; j < num_samples; j++) {
			diff_frames[i][j] = in_frames[i][j] - in_frames[i-1][j];
		}
	}
}

// Apply a filter in the time direction
void apply_frame_filters(float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, frameFilter** filters)
{
	for (int i = 0; i < NUM_FRAMES; i++) {
		for (int j = 0; j < num_samples; j++) {
			frameFilter_put(filters[j], in_frames[i][j]);
			filtered_frames[i][j] = frameFilter_get(filters[j]);
		}
	}
}

// Compute the signal RMS value in the time direction 
void apply_rms_filters(float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, rms_t** filters)
{
	for (int i = 0; i < NUM_FRAMES; i++) {
		for (int j = 0; j < num_samples; j++) {
			rms_put(rms[j], in_frames[i][j]);
			rms_frames[i][j] = rms_get(rms[j]);
		}
	}
}

// TBD
// 2-D RMS 15x5
// void apply_2d_rms(float rms_2d_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index, rms_t** filters)
// {
// 	static float temp_frames[NUM_FRAMES][MAX_FRAME_SIZE];
// 	for (int i = 0; i < num_samples; i++) {
// 		rms_put(rms[i], in_frames[frame_index][i]);
// 		rms_frames[frame_index][i] = rms_get(rms[i]);
// 	}
// }

// Find the max value in the spatial direction
void find_max(float max_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int max_indices[NUM_FRAMES])
{
	for (int i = 0; i < NUM_FRAMES; i++) {
		float max = in_frames[i][0];
		max_indices[i] = 0;
		for (int j = 1; j < num_samples; j++) {
			if (max < in_frames[i][j]) {
				max = in_frames[i][j];
				max_indices[i] = j;
			}
		}
		max_frames[i][max_indices[i]] = 1.0;
	}
}

// Find frequencies in the time direction
void compute_zeroxing(float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, float sampling_rate, zeroxing_t** zeroxing)
{
	for (int i = 0; i < NUM_FRAMES; i++) {
		for (int j = 0; j < num_samples; j++) {
			if (i >= 1) {
				freq_frames[i][j] = freq_frames[i-1][j]; // keep last frequency found
			}
			if (rms_frames[i][j] > RMS_THRESHOLD) {
				int is_valid = zeroxing_put(zeroxing[j], in_frames[i][j]);
				if (is_valid) {
					freq_frames[i][j] = zeroxing_get(zeroxing[j], sampling_rate);
				}
			}
			else {
				zeroxing_init(zeroxing[j]);
				freq_frames[i][j] = 0;
			}
		}
	}
}

// Get frequency at max RMSabsf(a_frames[i][j] + absf(
void find_frequency(float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int max_indices[NUM_FRAMES], 
	float frequencies[NUM_FRAMES], int valid_freq[NUM_FRAMES])
{
	for (int i = 0; i < NUM_FRAMES; i++) {
		valid_freq[i] = 0;
		frequencies[i] = 0.0;

		if (max_indices[i] == 0) continue;
		frequencies[i] = freq_frames[i][max_indices[i]];
		valid_freq[i] = 1;
	}
}

// Convolution in the space domain
void  convolution(float out_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, float mask[FILTER_SIZE])
{
	for (int i = 0; i < NUM_FRAMES; i++) {
        for (int j = 0; j < FILTER_SIZE/2; j++) {
            out_frames[i][j] = 0.0;
        }
        for (int j = FILTER_SIZE/2; j < num_samples - FILTER_SIZE/2; j++) {
            float sum = 0.0;
            for (int k = -FILTER_SIZE/2, m = 0; k < FILTER_SIZE/2; k++, m++) {
                sum += mask[m]*in_frames[i][j+k];
            }
            out_frames[i][j] = sum;
        }
        for (int j = num_samples - FILTER_SIZE/2; j < num_samples; j++) {
            out_frames[i][j] = 0.0;
        }
	}
}


void  add_absolute(float out_frames[NUM_FRAMES][MAX_FRAME_SIZE], float a_frames[NUM_FRAMES][MAX_FRAME_SIZE], float b_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples)
{
	for (int i = 0; i < NUM_FRAMES; i++) {
        for (int j = 0; j < num_samples; j++) {
            out_frames[i][j] = fabsf(a_frames[i][j] + fabsf(b_frames[i][j]));
        }
	}
}
