/*
 * breath1.cpp
 * Find respiration rate continuously wih SLMX4 sensor
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
#include "spatialFilter.h"
#include "breathFilter.h"
#include "zeroxing.h"

#define MAX_FRAME_SIZE 188

// constants associated with extract_breath_signal()
#define MAX_THRESHOLD 0.001
#define INVALID 0
#define VALID 1
#define LOST 2
#define LOST_COUNTER_MAX 8
#define CENTER_GAP 3

#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

slmx4 sensor;
timeOut timer;
float acquisition_frame[MAX_FRAME_SIZE*2];
float raw_frame[2][MAX_FRAME_SIZE];
float diff_frame[MAX_FRAME_SIZE];
float filtered_td_frame[MAX_FRAME_SIZE];
float filtered_sd_frame[MAX_FRAME_SIZE];

char  display[MAX_FRAME_SIZE+1];

float frequency;
float previous_freq;
int   valid;
float raw_breath_signal;
float filtered_breath_signal;
unsigned long int times;
int   new_frame_index;
int   old_frame_index;
float sampling_rate;

frameFilter   _filters_td_mem[MAX_FRAME_SIZE];
frameFilter*   filters_td[MAX_FRAME_SIZE];
spatialFilter _filter_sd_mem;
spatialFilter* filter_sd;
breathFilter  _filter_br_mem;
breathFilter*  filter_br;
zeroxing_t    _zeroxing_mem;
zeroxing_t*    zeroxing;


void  difference(float diff_frames[MAX_FRAME_SIZE], float new_frame[MAX_FRAME_SIZE], float old_frame[MAX_FRAME_SIZE], int num_samples);
void  apply_frame_filters(float filtered_frame[MAX_FRAME_SIZE], float in_frames[MAX_FRAME_SIZE], int num_samples, frameFilter** filters);
void  extract_breath_signal(float* breath_signal, int* valid, float in_frame[MAX_FRAME_SIZE], int num_samples);
void  apply_spatial_filter(float out_frames[MAX_FRAME_SIZE], float in_frames[MAX_FRAME_SIZE], int num_samples, spatialFilter* filter);
void  apply_breath_filter(float* out_signal, float in_signal, breathFilter* filter);
void  find_frequency(float* frequency, float previous_freq, float breath_signal, int valid, float sampling_rate, zeroxing_t* zeroxing);

void  display_slmx4_status();
char* display_signal(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE]);
void  clean_up(int sig);
void  launch_viewer();
void  swap(int* a, int* b);
int   find_min(float in_frame[MAX_FRAME_SIZE], int n, float* min, int* index);
int   find_max(float in_frame[MAX_FRAME_SIZE], int n, float* max, int* index);

const char* valid_str(int valid);
// int   write_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE], slmx4* sensor);
// int   read_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE]);
// int   write_sig_file(const char* filename, int valid[NUM_FRAMES], int max_indices[NUM_FRAMES], float breath_signal[NUM_FRAMES], int all);


int main(int argc, char* argv[])
{
    int show_siqnal = argc >= 2;
    int first = 1;

	// unlink(DATA_FILE_NAME);
	// if (argc == 1) {
	// 	mkfifo(DATA_FILE_NAME, FIFO_MODE);
	// 	launch_viewer();
	// }

	// Execute clean_up on CTRL_C
	signal(SIGINT, clean_up);

	// Initialise static memory
	memset(acquisition_frame,      0, MAX_FRAME_SIZE*2*sizeof(float));
	memset(raw_frame[0],           0, MAX_FRAME_SIZE*sizeof(float));
	memset(raw_frame[1],           0, MAX_FRAME_SIZE*sizeof(float));
	memset(diff_frame,             0, MAX_FRAME_SIZE*sizeof(float));
	memset(filtered_td_frame,      0, MAX_FRAME_SIZE*sizeof(float));
	memset(filtered_sd_frame,      0, MAX_FRAME_SIZE*sizeof(float));

    frequency = 0.0;
    previous_freq = 0.0;
    valid = 0;
    raw_breath_signal = 0.0;
    filtered_breath_signal = 0.0;
    new_frame_index = 0;
    old_frame_index = 1;

	// Use static memory for the various filter. Initialise filter buffers
	for (int i = 0; i < MAX_FRAME_SIZE; i++) {
		filters_td[i] = &_filters_td_mem[i];
		frameFilter_init(filters_td[i]);
	}

    filter_sd = &_filter_sd_mem;
    spatialFilter_init(filter_sd);

    filter_br = &_filter_br_mem;
    breathFilter_init(filter_br);

	zeroxing  = &_zeroxing_mem;
	zeroxing_init(zeroxing);

	// for (int i = 0; i < MAX_FRAME_SIZE; i++) {
	// 	rms[i] = &_rms_mem[i];
	// 	rms_init(rms[i]);
	// }

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
    sensor.set_value_by_name("VarSetValue_ByName(pps,64)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_min,896)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_max,1152)");
    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");
	display_slmx4_status();

	// Acquire data
	printf("Data aquisition starts\n");

    while(1) {
    	timer.initTimer();

		sensor.get_frame_normalized(acquisition_frame, POWER_IN_WATT);
		sampling_rate = 1000.0 / timer.elapsedTime_ms();
		memcpy(raw_frame[new_frame_index], acquisition_frame, sensor.get_num_samples()*sizeof(float));

    	difference(diff_frame, raw_frame[new_frame_index], raw_frame[old_frame_index], sensor.get_num_samples());
        swap(&new_frame_index, &old_frame_index);

        if (first) {
            first = 0; 
            continue;
        }

        apply_spatial_filter(filtered_sd_frame, diff_frame, sensor.get_num_samples(), filter_sd);
	    apply_frame_filters(filtered_td_frame, filtered_sd_frame, sensor.get_num_samples(), filters_td);
        extract_breath_signal(&raw_breath_signal, &valid, filtered_td_frame, sensor.get_num_samples());
        apply_breath_filter(&filtered_breath_signal, raw_breath_signal, filter_br);
        find_frequency(&frequency, previous_freq, filtered_breath_signal, valid, sampling_rate, zeroxing);
        previous_freq = frequency;

        if (show_siqnal)
            printf("%4.2f %7s %0.4f %s\n", sampling_rate, valid_str(valid), frequency, display_signal(display, filtered_td_frame));
        else
            printf("%4.2f %7s %0.4f\n", sampling_rate, valid_str(valid), frequency);
    }

	clean_up(0);
	return EXIT_SUCCESS;
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
void  difference(float diff_frame[MAX_FRAME_SIZE], float new_frame[MAX_FRAME_SIZE], float old_frame[MAX_FRAME_SIZE], int num_samples)
{
    for (int j = 0; j < num_samples; j++) {
        diff_frame[j] = new_frame[j] - old_frame[j];
    }
}

// Apply a filter in the time direction
void apply_frame_filters(float filtered_frame[MAX_FRAME_SIZE], float in_frame[MAX_FRAME_SIZE], int num_samples, frameFilter** filters)
{
    for (int j = 0; j < num_samples; j++) {
        frameFilter_put(filters[j], in_frame[j]);
        filtered_frame[j] = frameFilter_get(filters[j]);
    }
}

// Apply a filter in the spatial direction
void  apply_spatial_filter(float filtered_frame[MAX_FRAME_SIZE], float in_frame[MAX_FRAME_SIZE], int num_samples, spatialFilter* filter)
{
    spatialFilter_init(filter);
    for (int j = 0; j < num_samples; j++) {
        spatialFilter_put(filter, in_frame[j]);
        if (j >= SPATIALFILTER_TAP_NUM/2)
        filtered_frame[j-SPATIALFILTER_TAP_NUM/2] = spatialFilter_get(filter);
    }
}

// Apply a filter to the breath signal
void apply_breath_filter(float* out_signal, float in_signal, breathFilter* filter)
{
    breathFilter_put(filter, in_signal);
    *out_signal = breathFilter_get(filter);
}


// Extract breath signal from frames 
void extract_breath_signal(float* breath_signal,  int* valid, float in_frame[MAX_FRAME_SIZE], int num_samples)
{
    // Caution: this function should not be used in a multithread context
    static int tracking = 0;            // true if algorithm is in tracking mode
    static int lost_counter = 0;        // number of frame in lost mode
    static int last_valid_position = 0; // last position in valid mode    
    static int last_negative = 0;       // true if value found at last position was negative

    // Find max of absolute value in spatial domain
    float value, min, max;
    int position, pos_min, pos_max, has_min, has_max;

    has_min = find_min(in_frame, num_samples, &min, &pos_min);
    has_max = find_max(in_frame, num_samples, &max, &pos_max);
    if (has_min && has_max) {
        if (-min > max) {
            value = min;
            position = pos_min;
        }
        else {
            value = max;
            position = pos_max;
        }
    }
    else if (has_max) {
        value = max;
        position = pos_max;
    } 
    else if (has_min) {
        value = min;
        position = pos_min;
    }
    else {
        value = 0.0;
        position = 0;
    }   


// **** ICI - revoir avec le nouvel algo pour le max

    *valid = INVALID;
    *breath_signal = 0.0;

    // A max above MAX_THRESHOLD was found in this frame
    if (position) { 

        // If we are not in tracking mode, then accept signal at this position
        // an go in tracking mode
        if (!tracking) {        
            *valid = VALID;
            tracking = 1;
            *breath_signal = in_frame[position];
            last_valid_position = position;
        }

        // If we are in already tracking mode, perform a position continuity test 
        // before accepting the signal. 
        // Stay in tracking mode for at most LOST_COUNTER_MAX frames
        else {
            // Test if the sign has changed
 
            // Test if if max position is too far from the previous one
            if (position > last_valid_position + CENTER_GAP || position < last_valid_position - CENTER_GAP) {

                // The position is too far. Prepare to go in LOST mode. 
                lost_counter++;

                // Keep the ref position for at most LOST_COUNTER_MAX
                if (lost_counter <= LOST_COUNTER_MAX) {
                    *valid = LOST;
                    position = last_valid_position;
                    *breath_signal = in_frame[position];
                }
                // We have been lost for too many frames, reset the search
                else {
                    *valid = INVALID;
                    lost_counter = 0;
                    tracking = 0;
                }
            }

            // This max position is close to the previous one, we're OK
            else {
                *valid = VALID;
                *breath_signal = in_frame[position];
                last_valid_position = position;
                lost_counter = 0;
            }
        }
    }

    // A max above MAX_THRESHOLD was not found in this frame
    else {
        // Keep the previous position only if one exists
        if (tracking) {
            // Prepare to go in LOST mode. 
            lost_counter++;

            // Keep the previous position for at most LOST_COUNTER_MAX
            if (lost_counter <= LOST_COUNTER_MAX) {
                *valid = LOST;
                position = last_valid_position;
                *breath_signal = in_frame[position];
            }

            // We have been in lost mode for too much time, reset the search
            else {
                *valid = INVALID;
                lost_counter = 0;
                tracking = 0;
            }
        }
    }
}


// Find frequencies from the breath signal
void  find_frequency(float* frequency, float previous_freq, float in_signal, int valid, float sampling_rate, zeroxing_t* zeroxing)
{
    if (valid == INVALID) {
        *frequency = 0.0;
        zeroxing_init(zeroxing);
    }
    else {
        *frequency = previous_freq; // keep last frequency found
        int found = zeroxing_put(zeroxing, in_signal);
        if (found) {
            *frequency = zeroxing_get(zeroxing, sampling_rate);
        }
    }
}

void  swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

const char* valid_str(int valid) 
{
    switch(valid) {
        case INVALID: return "I";
        case VALID:   return "V";
        case LOST:    return "?";
    }
    return "";
}

char* display_signal(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE])
{
    const static char* t = "X97531:. ,;02468M";
    int   x;
    float max = 0.04;

    for (int i = 0; i < MAX_FRAME_SIZE; i++) {
        x = round(in[i] * 8.0 / max) + 8;
        x = x >  16 ? 16 : x;
        x = x <   0 ?  0 : x;
        out[i] = t[x];
    }
    out[MAX_FRAME_SIZE] = '\0';

    return out;
}


int  fsign(float x)
{
    if (x > 0.0) return 1;
    if (x < 0.0) return -1;
    return 0;
}

int find_min(float in_frame[MAX_FRAME_SIZE], int n, float* min, int* index)
{
    int found = 0;
    *min = 0.0;
    *index = 0;

    for (int j = 1; j < n; j++) {
        if (in_frame[j] > -MAX_THRESHOLD) continue;
        if (in_frame[j] < *min) {
            *min = in_frame[j]; 
            *index = j;
            found = 1;
        }
    }
    return found;
}

int find_max(float in_frame[MAX_FRAME_SIZE], int n, float* max, int* index)
{
    int found = 0;
    *max = 0.0;
    *index = 0;

    for (int j = 1; j < n; j++) {
        if (in_frame[j] < MAX_THRESHOLD) continue;
        if (in_frame[j] > *max) {
            *max = in_frame[j]; 
            *index = j;
            found = 1;
        }
    }
    return found;
}