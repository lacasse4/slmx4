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
#define WINDOW_SIZE 5
#define RMS_THRESHOLD 0.005    // experimentaly set with "DATA-iter64-pps128-rms-141ms"

#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

slmx4 sensor;
timeOut timer;
float acquisition_frame[MAX_FRAME_SIZE*2];
float raw_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float max_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE];
unsigned long int times[NUM_FRAMES];

frameFilter _filters_mem[MAX_FRAME_SIZE];
frameFilter* filters[MAX_FRAME_SIZE];

rms_t _rms_mem[MAX_FRAME_SIZE];
rms_t* rms[MAX_FRAME_SIZE];

zeroxing_t _zeroxing_mem[MAX_FRAME_SIZE];
zeroxing_t* zeroxing[MAX_FRAME_SIZE];


void  difference(float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, int frame_index);
void  moving_average(float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, int frame_index, int window_size);
void  apply_frame_filters(float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, int frame_index, frameFilter** filters);
void  apply_rms_filters(float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, int frame_index, rms_t** rms);
void  find_max(float max_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, int frame_index);
void  compute_zeroxing(float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, int frame_index, float sampling_rate, zeroxing_t** zeroxing);

void  display_slmx4_status();
void  clean_up(int sig);
void  launch_viewer();
int   write_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE], slmx4* sensor);


int main(int argc, char* argv[])
{
	// peak_t peak;

	// unlink(DATA_FILE_NAME);
	// if (argc == 1) {
	// 	mkfifo(DATA_FILE_NAME, FIFO_MODE);
	// 	launch_viewer();
	// }

	signal(SIGINT, clean_up);

	memset(acquisition_frame, 0, MAX_FRAME_SIZE*2*sizeof(float));
	memset(raw_frames,        0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(diff_frames,       0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(filtered_frames,   0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(rms_frames,        0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(max_frames,        0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(freq_frames,       0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));

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


	printf("Open serial port: %s\n", SERIAL_PORT);
	if (access(SERIAL_PORT, F_OK) < 0) {
		fprintf(stderr, "ERROR: serial port NOT available: %s\n", SERIAL_PORT);		
		exit(EXIT_FAILURE);
	}

	printf("Initialize SLMX4 sensor\n");
	if (sensor.begin(SERIAL_PORT) == EXIT_FAILURE) {
		exit(EXIT_FAILURE);
	}

    sensor.set_value_by_name("VarSetValue_ByName(iterations,64)");
    sensor.set_value_by_name("VarSetValue_ByName(pps,128)");

//    sensor.set_value_by_name("VarSetValue_ByName(fs,2.9)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_min,896)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_max,1152)");
    // sensor.set_value_by_name("VarSetValue_ByName(frame_start,2.0)");
    // sensor.set_value_by_name("VarSetValue_ByName(frame_end,4.0)");
    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");
	display_slmx4_status();


	// acquire data
    float sampling_frequency = 0.0;
	printf("Data aquisition starts\n");
	for (int i = 0; i < NUM_FRAMES; i++) {
    	timer.initTimer();

		sensor.get_frame_normalized(acquisition_frame, POWER_IN_WATT);
		memcpy(raw_frames[i], acquisition_frame, sensor.get_num_samples()*sizeof(float));
		if (i >= 1) difference(diff_frames, raw_frames, sensor.get_num_samples(), i);
		apply_frame_filters(filtered_frames, diff_frames, sensor.get_num_samples(), i, filters);
        apply_rms_filters(rms_frames, filtered_frames, sensor.get_num_samples(), i, rms);
        compute_zeroxing(freq_frames, filtered_frames, rms_frames, sensor.get_num_samples(), i, sampling_frequency, zeroxing);
        find_max(max_frames, rms_frames, sensor.get_num_samples(), i);

		times[i] = timer.elapsedTime_ms();
        sampling_frequency = 1000.0 / times[i];
	}
	printf("Data acquisition ends\n");

	write_file("RAW_FRAMES", raw_frames, &sensor);
	write_file("DIFF_FRAMES", diff_frames, &sensor);
	write_file("FILTERED_FRAMES", filtered_frames, &sensor);
	write_file("RMS_FRAMES", rms_frames, &sensor);
	write_file("MAX_FRAMES", max_frames, &sensor);
	write_file("FREQ_FRAMES", freq_frames, &sensor);
	printf("Files written\n");

	float average = 0.0;
	for (int i = 0; i < NUM_FRAMES; i++) {
		average += times[i];
	}
	printf("Average acquisiton time = %f ms\n", average/NUM_FRAMES);

	clean_up(0);

	return EXIT_SUCCESS;
}

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

void clean_up(int sig)
{
	printf("\nClose sensor\n");
	sensor.end();
	// unlink(DATA_FILE_NAME);
	exit(EXIT_SUCCESS);
}

void launch_viewer()
{
	int pid = fork();
	if (pid == 0) {
		execlp("python", "python", "plotdiff.py", NULL);
		fprintf(stderr, "ERROR - Unable to start viewer\n");
		exit(EXIT_FAILURE);
	}
}


void  difference(float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index)
{
	int i;

	for (i = 0; i < num_samples; i++) {
		diff_frames[frame_index][i] = in_frames[frame_index][i] - in_frames[frame_index-1][i];
	}
}


void moving_average(float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index, int window_size)
{
	int i, j;
	float sum;

	for (i = 0; i < num_samples; i++) {
		sum = 0.0;
		for (j = 0; j < window_size; j++) {
			sum += in_frames[frame_index-j][i];
		}
		filtered_frames[frame_index][i] = sum / window_size;
	}
}

void apply_frame_filters(float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index, frameFilter** filters)
{
	for (int i = 0; i < num_samples; i++) {
		frameFilter_put(filters[i], in_frames[frame_index][i]);
		filtered_frames[frame_index][i] = frameFilter_get(filters[i]);
	}
}


void apply_rms_filters(float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index, rms_t** filters)
{
	for (int i = 0; i < num_samples; i++) {
		rms_put(rms[i], in_frames[frame_index][i]);
		rms_frames[frame_index][i] = rms_get(rms[i]);
	}
}

void find_max(float max_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index)
{
    float max = in_frames[frame_index][0];
    int max_index = 0;
	for (int i = 1; i < num_samples; i++) {
        if (max < in_frames[frame_index][i]) {
            max = in_frames[frame_index][i];
            max_index = i;
        }
	}
    max_frames[frame_index][max_index] = 1.0;
}

void compute_zeroxing(float freq_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], float rms_frames[NUM_FRAMES][MAX_FRAME_SIZE], 
    int num_samples, int frame_index, float sampling_rate, zeroxing_t** zeroxing)
{
	for (int i = 0; i < num_samples; i++) {
        if (frame_index >= 1) {
            freq_frames[frame_index][i] = freq_frames[frame_index-1][i];
        }
        if (rms_frames[frame_index][i] > RMS_THRESHOLD) {
		    int is_valid = zeroxing_put(zeroxing[i], in_frames[frame_index][i]);
            if (is_valid) {
                freq_frames[frame_index][i] = zeroxing_get(zeroxing[i], sampling_rate);
            }
        }
		else {
			zeroxing_init(zeroxing[i]);
			freq_frames[frame_index][i] = 0;
		}
	}
}
