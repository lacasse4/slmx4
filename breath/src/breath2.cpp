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
#define MIN_VALID_SIGNAL 0.001
#define MAX_VALID_SLOPE 0.02
#define INVALID 0
#define VALID 1
#define LOST 2
#define LOST_COUNTER_MAX 14
#define POSITION_GAP 5
#define DISPLAY_RANGE 0.03
#define MAX_BREATH_SLOPE 2.0

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

int   position_debug;
int   slope_debug;
int   gap_debug;
int   lost_debug;
int   breath_debug;
float in_frame_debug;
float last_value_debug;

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
char* display_char(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE], int index, int position);

void  clean_up(int sig);
void  launch_viewer();
void  swap(int* a, int* b);
int   find_min(float in_frame[MAX_FRAME_SIZE], int n);
int   find_max(float in_frame[MAX_FRAME_SIZE], int n);

const char* valid_str(int valid);
// int   write_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE], slmx4* sensor);
// int   read_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE]);
// int   write_sig_file(const char* filename, int valid[NUM_FRAMES], int max_indices[NUM_FRAMES], float breath_signal[NUM_FRAMES], int all);


int main(int argc, char* argv[])
{
    int show_siqnal = argc >= 2;
    int first = 1;

	// unlink(DATA_FILE_NAME);display = 
	// if (argc == 1) {
	// 	mkfifo(DATA_FILE_NAME, FIFO_MODE);
	// 	launch_viewer();
	// }

	// Execute clean_up on CTRL_C
	signal(SIGINT, clean_up);

	// Initialise static memory
	memset(acquisition_frame, 0, MAX_FRAME_SIZE*2*sizeof(float));
	memset(raw_frame[0],      0, MAX_FRAME_SIZE*sizeof(float));
	memset(raw_frame[1],      0, MAX_FRAME_SIZE*sizeof(float));
	memset(diff_frame,        0, MAX_FRAME_SIZE*sizeof(float));
	memset(filtered_td_frame, 0, MAX_FRAME_SIZE*sizeof(float));
	memset(filtered_sd_frame, 0, MAX_FRAME_SIZE*sizeof(float));

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
	//   - 32 iterations
	//   - 64 pulses per step (pps)
	//   - dac minimum value 896   (1024 - 128)
	//   - dac maximum value 1152  (1024 + 128)
	//   - downconversion and decimation enabled (ddc = 1)
	// The 4 first values were experimentaly set.
	// The iteration and pps values yield a smooth signal (less noise)
	// but increase the data acquisition time between frames.
	// By increasing dac_min and reducing dac_max, the data acquisition
	// time is reduced. This application does not require a larger range.
	// With this setup, the acquition rate is about 1 frame each 37 ms,
	// or 27.0 frames per second. 
    // The frameFilter.c is designed for this rate.
	//
	// Setting ddc to 1 enable the downconverion and decimation algorithm 
	// available on the SLMX4 sensor. It demodulates the radar signal 
	// from its carrier frequency. See:
	// https://sensorlogicinc.github.io/modules/docs/XTAN-13_XeThruX4RadarUserGuide_rev_a.pdf section 5.
	// The result is 188 points in complex numbers (float*2). 
	// Each complex data points is rapidely converted to there modulus 
	// in get_frame_normalized().   

    sensor.set_value_by_name("VarSetValue_ByName(iterations,32)");
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

        if (show_siqnal) {
            printf("%5.2f %s ", frequency * 60, valid_str(valid));

            printf("%c",  slope_debug   ? 'S' : ' ');
            printf("%c",  gap_debug     ? 'G' : ' ');
            printf("%c ", lost_debug    ? 'L' : ' ');
            printf("%c ", breath_debug  ? 'B' : ' ');

            for (int i = 0; i < MAX_FRAME_SIZE/2; i++) printf("%s", display_char(display, filtered_td_frame, i, position_debug));
            printf(" %7.4f", filtered_td_frame[position_debug]);
            printf("\n");
        }
        else {
            printf("%4.2f %s\n", frequency, valid_str(valid));
        }
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
    // Caution: this function should not be used in a multithreading context
    static int   tracking = 0;          // true if algorithm is in tracking mode
    static int   lost_counter = 0;      // number of frame in lost mode
    static int   last_position = 0;     // last position in valid mode    
    static float last_value = 0;        // signal value of the previous frame

    int position;                       // index of the max absolute value in in_frame[]
    int pos_min, pos_max;

    slope_debug = 0;
    gap_debug = 0;
    lost_debug = 0;

    // Find min and max of the frame (above MIN_VALID_SIGNAL)
    pos_min = find_min(in_frame, num_samples);
    pos_max = find_max(in_frame, num_samples);

    // Find the position of greatest min or max in frame
    position = 0;
    if (pos_min && pos_max) {
        position = -in_frame[pos_min] > in_frame[pos_max] ? pos_min : pos_max;
    }
    else if (pos_min) {
        position = pos_min;
    } 
    else if (pos_max) {
        position = pos_max;
    }

    // Prepare to extract the breath signal. At start, assume this frame is invalid.
    *valid = INVALID;
    *breath_signal = 0.0;

    if (position) { 
        // A value above MIN_VALID_SIGNAL was found in this frame

        if (!tracking) {        
            // We are not in tracking mode and data is valid.  
            // Accept this data. Go in tracking mode.
            tracking = 1;
            lost_counter = 0;
            last_position = position;
            last_value = in_frame[position];

            *valid = VALID;
            *breath_signal = in_frame[position];
        }

        else {
            // We are tracking mode.
            // Perform few tests before accepting data. 

            // Check if signal amplitude is within an acceptable range from the last frame
            if (fabsf(in_frame[position] - last_value) > MAX_VALID_SLOPE) {
                // The signal's slope is to steep to be breath data.
                // Data is invalid.  Step out of tracking mode.
                tracking = 0;
                lost_counter = 0;
                last_position = 0;
                last_value = 0.0;

                *valid = INVALID;
                *breath_signal = 0.0;

                slope_debug = 1;
            }
 
            // Check if position is too far from the last valid one.
            else if (position > last_position + POSITION_GAP) {
                // The max position is too far from the last valid one.
                // Data is not valid, but allow to progress blindly for at most LOST_COUNTER_MAX times

                // Check if we have exceeded LOST_COUNTER_MAX times
                if (lost_counter > LOST_COUNTER_MAX) {
                    // We have been progressing blindly for too many frames.
                    // Data is invalid.  Step out of tracking mode.
                    tracking = 0;
                    lost_counter = 0;
                    last_position = 0;
                    last_value = 0.0;

                    *valid = INVALID;
                    *breath_signal = 0.0;

                    gap_debug = 1;
                }
                else {
                    // We have not exceeded LOST_COUNTER_MAX times.
                    // Use position from last valid frame. Use value of current frame at this position.
                    // We are progressing blindly (LOST). Stay in tracking mode.
                    tracking = 1;
                    lost_counter++;
                    position = last_position + 1;
                    last_position++;
                    last_value = in_frame[position];

                    *valid = LOST;
                    *breath_signal = in_frame[position];

                    gap_debug = 1;
                }
            }

            else if (position < last_position - POSITION_GAP) {
                // The max position is too far from the last valid one.
                // Data is not valid, but allow to progress blindly for at most LOST_COUNTER_MAX times

                // Check if we have exceeded LOST_COUNTER_MAX times
                if (lost_counter > LOST_COUNTER_MAX) {
                    // We have been progressing blindly for too many frames.
                    // Data is invalid.  Step out of tracking mode.
                    tracking = 0;
                    lost_counter = 0;
                    last_position = 0;
                    last_value = 0.0;

                    *valid = INVALID;
                    *breath_signal = 0.0;

                    gap_debug = 1;
                }
                else {
                    // We have not exceeded LOST_COUNTER_MAX times.
                    // Use position from last valid frame. Use value of current frame at this position.
                    // We are progressing blindly (LOST). Stay in tracking mode.
                    tracking = 1;
                    lost_counter++;
                    position = last_position - 1;
                    last_position--;
                    last_value = in_frame[position];

                    *valid = LOST;
                    *breath_signal = in_frame[position];

                    gap_debug = 1;
                }
            }

            // This position is close enough to the previous one, use the new position data
            else {
                tracking = 1;
                lost_counter = 0;
                last_position = position;
                last_value = in_frame[position];

                *valid = VALID;
                *breath_signal = in_frame[position];
            }
        }
    }

    else {
        // A value above MIN_VALID_SIGNAL was not found in this frame
        if (tracking) {
            // We are in tracking mode

            // Check if we have exceeded LOST_COUNTER_MAX times
            if (lost_counter > LOST_COUNTER_MAX) {
                // We have been progressing blindly for too many frames.
                // Data is invalid.  Step out of tracking mode.
                tracking = 0;
                lost_counter = 0;
                last_position = 0;
                last_value = 0.0;

                *valid = INVALID;
                *breath_signal = 0.0;

                lost_debug = 1;
            }
            else {
                // Keep the previous position for at most LOST_COUNTER_MAX times
                // Use position from last valid frame. Use value of current frame at this position.
                // We are progressing blindly (LOST). Stay in tracking mode.
                tracking = 1;
                lost_counter++;
                position = last_position;
                last_value = in_frame[position];

                *valid = LOST;
                *breath_signal = in_frame[position];

                lost_debug = 1;
            }
        }
    }

    position_debug = position;
}


// Find frequencies from the breath signal
void  find_frequency(float* frequency, float previous_freq, float in_signal, int valid, float sampling_rate, zeroxing_t* zeroxing)
{
    breath_debug = 0;
    if (valid == INVALID) {
        *frequency = 0.0;
        zeroxing_init(zeroxing);
    }
    else {
        *frequency = previous_freq; // keep last frequency found
        int found = zeroxing_put(zeroxing, in_signal);
        if (found) {
            *frequency = zeroxing_get(zeroxing, sampling_rate);
            if (previous_freq > 0.0 && fabsf(previous_freq - *frequency)*60.0 > MAX_BREATH_SLOPE) {
                *frequency = 0.0;
                zeroxing_init(zeroxing);
                breath_debug = 1;
            }
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
        case INVALID: return "\e[101m \e[0m";
        case VALID:   return "\e[102m \e[0m";
        case LOST:    return "\e[103m \e[0m";
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

char* display_char(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE], int index, int position)
{
    char c = position == index ? '|' : ' ';

    int x = round(in[index] * 256 / DISPLAY_RANGE) + 128;
    x = x >  255 ? 255 : x;
    x = x <    0 ?   0 : x;

    sprintf(out, "\e[94m\e[48;2;%d;%d;%dm%c\e[0m", x, x, x, c);
    return out;
}


int  fsign(float x)
{
    if (x > 0.0) return 1;
    if (x < 0.0) return -1;
    return 0;
}

int find_min(float in_frame[MAX_FRAME_SIZE], int n)
{
    float min = 0.0;
    int index = 0;

    for (int j = 1; j < n; j++) {
        if (in_frame[j] > -MIN_VALID_SIGNAL) continue;
        if (in_frame[j] < min) {
            min = in_frame[j]; 
            index = j;
        }
    }
    return index;
}

int find_max(float in_frame[MAX_FRAME_SIZE], int n)
{
    float max = 0.0;
    int index = 0;

    for (int j = 1; j < n; j++) {
        if (in_frame[j] < MIN_VALID_SIGNAL) continue;
        if (in_frame[j] > max) {
            max = in_frame[j]; 
            index = j;
        }
    }
    return index;
}