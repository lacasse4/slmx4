/*
 * breath2.cpp
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
#include <arpa/inet.h>
#include <sys/select.h>


#include "slmx4_vcom.h"
#include "serialib.h"
// #include "frameFilter.h"
// #include "spatialFilter.h"
#include "breathFilter.h"
#include "zeroxing.h"
#include "tinyosc.h"
#include "peak.h"

#define MAX_FRAME_SIZE 188
#define MIN_DB  (-22.0)
#define RESOLUTION 0.0525   // sensor resolution for the senser in ddc mode in meter
#define MIN_DISTANCE (9*RESOLUTION) // in meter
#define MAX_DISTANCE 2.0    // in meter

// constants associated with extract_breath_signal()
#define MIN_VALID_SIGNAL 0.001
#define MAX_VALID_SLOPE 0.02
#define INVALID 0
#define VALID 1
#define LOST 2
#define LOST_COUNTER_MAX 14
#define POSITION_GAP 2
#define DISPLAY_RANGE 0.03
#define MAX_BREATH_SLOPE 3.0
    
#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

// typedef struct breath_point {
//     float signal;
//     float frequency;
//     int position;
//     int mode;
//     int freq_valid;
//     int is_slope;
//     int is_gap;
//     int is_lost;
//     int is_freq;
// } breath_point_t;

slmx4 sensor;
timeOut timer;
float frame[MAX_FRAME_SIZE*2];
// float raw_frame[2][MAX_FRAME_SIZE];
// float diff_frame[MAX_FRAME_SIZE];
// float filtered_td_frame[MAX_FRAME_SIZE];
// float filtered_sd_frame[MAX_FRAME_SIZE];

char  display[MAX_FRAME_SIZE+1];

float previous_freq;
unsigned long int times;
int   new_frame_index;
int   old_frame_index;
float sampling_rate;
struct sockaddr_in server_addr;
int   fd;

// frameFilter     _filters_td_mem[MAX_FRAME_SIZE];
// frameFilter*     filters_td[MAX_FRAME_SIZE];
// spatialFilter   _filter_sd_mem;
// spatialFilter*   filter_sd;
breathFilter    _filter_br_mem;
breathFilter*    filter_br;
zeroxing_t      _zeroxing_mem;
zeroxing_t*      zeroxing;
// breath_point_t  _breath_point_mem;
// breath_point_t*  breath_point;
peak_t           peak;

// void  difference(float diff_frames[MAX_FRAME_SIZE], float new_frame[MAX_FRAME_SIZE], float old_frame[MAX_FRAME_SIZE], int num_samples);
// void  apply_frame_filters(float filtered_frame[MAX_FRAME_SIZE], float in_frames[MAX_FRAME_SIZE], int num_samples, frameFilter** filters);
// void  extract_breath_signal(breath_point_t* breath_point, float in_frame[MAX_FRAME_SIZE], int num_samples);
// void  apply_spatial_filter(float out_frames[MAX_FRAME_SIZE], float in_frames[MAX_FRAME_SIZE], int num_samples, spatialFilter* filter);
void  apply_breath_filter(float* out_signal, float in_signal, breathFilter* filter);
// void  find_frequency(breath_point_t* breath_point, float previous_freq, float sampling_rate, zeroxing_t* zeroxing);

void  display_slmx4_status(FILE* f);
char* display_signal(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE]);
// char* display_char(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE], int index, breath_point_t* breath_point);

void  clean_up(int sig);
void  launch_viewer();
void  swap(int* a, int* b);
int   find_min(float in_frame[MAX_FRAME_SIZE], int n);
int   find_max(float in_frame[MAX_FRAME_SIZE], int n);

const char* mode_str(int valid);
// int   write_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE], slmx4* sensor);
// int   read_img_file(const char* filename, float frames[NUM_FRAMES][MAX_FRAME_SIZE]);
// int   write_sig_file(const char* filename, int valid[NUM_FRAMES], int max_indices[NUM_FRAMES], float breath_signal[NUM_FRAMES], int all);
void send_frequency_to_MaxMSP(struct sockaddr_in* server, float frequency);
void send_valid_to_MaxMSP(struct sockaddr_in* server, int valid);


int main(int argc, char* argv[])
{
    if (argc != 1 && argc != 3) {
        printf("Usage:\n");
        printf("       breath3 <ip_address> <port>  (with MaxMSP)\n");
        printf(" or    breath3                      (in debug mode)\n");
        return 1;
    }

    int debug = argc == 1;
    // int first = 1;
    fd = 0;

    if (!debug) {
        struct in_addr addr_binary;
        if (inet_pton(AF_INET, argv[1], &addr_binary) == 0) {
            fprintf(stderr, "Error: invalid ip address\n");
            return 1;
        };

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1) {
            fprintf(stderr, "Error: unable to open UDP connection\n");
            return 1;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr = addr_binary;
        server_addr.sin_port = htons(atoi(argv[2]));
    }

	// Execute clean_up on CTRL_C
	signal(SIGINT, clean_up);

	// Initialise static memory
	memset(frame, 0, MAX_FRAME_SIZE*2*sizeof(float));
	// memset(raw_frame[0],      0, MAX_FRAME_SIZE*sizeof(float));
	// memset(raw_frame[1],      0, MAX_FRAME_SIZE*sizeof(float));
	// memset(diff_frame,        0, MAX_FRAME_SIZE*sizeof(float));
	// memset(filtered_td_frame, 0, MAX_FRAME_SIZE*sizeof(float));
	// memset(filtered_sd_frame, 0, MAX_FRAME_SIZE*sizeof(float));

    previous_freq = 0.0;
    new_frame_index = 0;
    old_frame_index = 1;

	// Use static memory for the various data structures
    // and initialize memory
	// for (int i = 0; i < MAX_FRAME_SIZE; i++) {
	// 	filters_td[i] = &_filters_td_mem[i];
	// 	frameFilter_init(filters_td[i]);
	// }

    // filter_sd = &_filter_sd_mem;
    // spatialFilter_init(filter_sd);

    filter_br = &_filter_br_mem;
    breathFilter_init(filter_br);

	zeroxing  = &_zeroxing_mem;
	zeroxing_init(zeroxing);

    // breath_point = &_breath_point_mem;
    // memset(breath_point, 0, sizeof(breath_point_t));

	// Check if the SLMX4 sensor is connected and available
	fprintf(stderr, "Checking serial port: %s\n", SERIAL_PORT);
	if (access(SERIAL_PORT, F_OK) < 0) {
		fprintf(stderr, "ERROR: serial port NOT available: %s\n", SERIAL_PORT);		
		exit(EXIT_FAILURE);
	}

	// Initialise sensor
	fprintf(stderr, "Initializing SLMX4 sensor\n");
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
	// Each complex data point is converted to its modulus in get_frame_normalized().  
    // Each data point correspond to a distance of 0.0525 m (5.25 cm).


    sensor.set_value_by_name("VarSetValue_ByName(iterations,32)");
    sensor.set_value_by_name("VarSetValue_ByName(pps,64)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_min,896)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_max,1152)");
    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");
	display_slmx4_status(stderr);

	// Acquire data
	fprintf(stderr, "Breath frequency detection in progress. Type CTRL_C to exit.\n");

    while(1) {
    	timer.initTimer();
		sensor.get_frame_normalized(frame, POWER_IN_WATT);
		sampling_rate = 1000.0 / timer.elapsedTime_ms();
		// memcpy(raw_frame[new_frame_index], acquisition_frame, sensor.get_num_samples()*sizeof(float));

    	// difference(diff_frame, raw_frame[new_frame_index], raw_frame[old_frame_index], sensor.get_num_samples());
        // swap(&new_frame_index, &old_frame_index);

        // if (first) {
        //     first = 0; 
        //     continue;
        // }

        // apply_spatial_filter(filtered_sd_frame, diff_frame, sensor.get_num_samples(), filter_sd);
	    // apply_frame_filters(filtered_td_frame, filtered_sd_frame, sensor.get_num_samples(), filters_td);
        // extract_breath_signal(breath_point, filtered_td_frame, sensor.get_num_samples());

        // peak = find_first_peak_above(frame, MAX_FRAME_SIZE, 0.0, 9.8, MIN_DISTANCE, MAX_DISTANCE, MIN_DB); 
        peak = find_highest_peak(frame, MAX_FRAME_SIZE, 0.0, 9.8, MIN_DISTANCE, MAX_DISTANCE); 
        // apply_breath_filter(&peak.power, peak.power, filter_br);
        // find_frequency(breath_point, previous_freq, sampling_rate, zeroxing);

        if (debug) {
            if (peak.index == -1) fprintf(stderr, "E");
            printf("%7.4f\n", peak.distance);

            // printf(" %s ", mode_str(breath_point->mode));
            // printf("%c",  breath_point->is_slope   ? 'S' : ' ');
            // printf("%c",  breath_point->is_gap     ? 'G' : ' ');
            // printf("%c ", breath_point->is_lost    ? 'L' : ' ');
            // printf("%c ", breath_point->is_freq    ? 'F' : ' ');

            // for (int i = 0; i < MAX_FRAME_SIZE/2; i++) printf("%s", display_char(display, filtered_td_frame, i, breath_point));
            // printf("\n");
        }
        // else {
        //     if (previous_freq != breath_point->frequency) {
        //         send_frequency_to_max(&server_addr, breath_point->frequency * 60.0f);
        //     }
        // }
        // previous_freq = breath_point->frequency;
    }

	clean_up(0);
	return EXIT_SUCCESS;
}

// Ouput slmx4 status to console
void display_slmx4_status(FILE *f) 
{
	fprintf(f, "dac_min           = %d\n", sensor.get_dac_min());
	fprintf(f, "dac_max           = %d\n", sensor.get_dac_max());
	fprintf(f, "dac_step          = %d\n", sensor.get_dac_step());
	fprintf(f, "pps               = %d\n", sensor.get_pps());
	fprintf(f, "iterations        = %d\n", sensor.get_iterations());
	fprintf(f, "prf_div           = %d\n", sensor.get_prf_div());
	fprintf(f, "prf               = %e\n", sensor.get_prf());
	fprintf(f, "fs                = %e\n", sensor.get_fs());
	fprintf(f, "num_samples       = %d\n", sensor.get_num_samples());
	fprintf(f, "frame_length      = %d\n", sensor.get_frame_length());
	fprintf(f, "rx_wait           = %d\n", sensor.get_rx_wait());
	fprintf(f, "tx_region         = %d\n", sensor.get_tx_region());
	fprintf(f, "tx_power          = %d\n", sensor.get_tx_power());
	fprintf(f, "ddc_en            = %d\n", sensor.is_ddc_en());
	fprintf(f, "frame_offset      = %e\n", sensor.get_frame_offset());
	fprintf(f, "frame_start       = %e\n", sensor.get_frame_start());
	fprintf(f, "frame_end         = %e\n", sensor.get_frame_end());
	fprintf(f, "sweep_time        = %e\n", sensor.get_sweep_time());
	fprintf(f, "unambiguous_range = %e\n", sensor.get_unambiguous_range());
	fprintf(f, "res               = %e\n", sensor.get_res());
	fprintf(f, "fs_rf             = %e\n", sensor.get_fs_rf());
}

// Must be executed before exit in order to leave the sensor in a stable mode
void clean_up(int sig)
{
	fprintf(stderr, "\nClose sensor\n");
	sensor.end();
    if (fd != 0) close(fd);
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
// void apply_frame_filters(float filtered_frame[MAX_FRAME_SIZE], float in_frame[MAX_FRAME_SIZE], int num_samples, frameFilter** filters)
// {
//     for (int j = 0; j < num_samples; j++) {
//         frameFilter_put(filters[j], in_frame[j]);
//         filtered_frame[j] = frameFilter_get(filters[j]);
//     }
// }

// Apply a filter in the spatial direction
// void  apply_spatial_filter(float filtered_frame[MAX_FRAME_SIZE], float in_frame[MAX_FRAME_SIZE], int num_samples, spatialFilter* filter)
// {
//     spatialFilter_init(filter);
//     for (int j = 0; j < num_samples; j++) {
//         spatialFilter_put(filter, in_frame[j]);
//         if (j >= SPATIALFILTER_TAP_NUM/2)
//             filtered_frame[j-SPATIALFILTER_TAP_NUM/2] = spatialFilter_get(filter);
//     }
// }

// Apply a filter to the breath signal
void apply_breath_filter(float* out_signal, float in_signal, breathFilter* filter)
{
    breathFilter_put(filter, in_signal);
    *out_signal = breathFilter_get(filter);
}


// Extract breath signal from frames 
/*
void extract_breath_signal(breath_point_t* breath_point, float in_frame[MAX_FRAME_SIZE], int num_samples)
{
    // Caution: this function should not be used in a multithreading context
    static int   tracking = 0;          // true if algorithm is in tracking mode
    static int   lost_counter = 0;      // number of frame in lost mode
    static int   last_position = 0;     // last position in valid mode    
    static float last_value = 0;        // signal value of the previous frame

    int position;                       // index of the max absolute value in in_frame[]
    int pos_min, pos_max;

    breath_point->is_slope = 0;
    breath_point->is_gap   = 0;
    breath_point->is_lost  = 0;

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
    breath_point->mode = INVALID;
    breath_point->signal = 0.0;

    if (position) { 
        // A value above MIN_VALID_SIGNAL was found in this frame

        if (!tracking) {        
            // We are not in tracking mode and data is valid.  
            // Accept this data. Go in tracking mode.
            tracking = 1;
            lost_counter = 0;
            last_position = position;
            last_value = in_frame[position];

            breath_point->mode = VALID;
            breath_point->signal = in_frame[position];
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

                breath_point->mode = INVALID;
                breath_point->signal = 0.0;
                breath_point->is_slope = 1;
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

                    breath_point->mode = INVALID;
                    breath_point->signal = 0.0;
                    breath_point->is_gap = 1;
                }
                else {
                    // We have not exceeded LOST_COUNTER_MAX times.
                    // Use position from last valid frame. Use value of current frame at this position.
                    // We are progressing blindly (LOST). Stay in tracking mode.
                    tracking = 1;
                    lost_counter++;
                    position = last_position;
                    last_position++;
                    last_value = in_frame[position];

                    breath_point->mode = LOST;
                    breath_point->signal = in_frame[position];
                    breath_point->is_gap = 1;
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

                    breath_point->mode = INVALID;
                    breath_point->signal = 0.0;
                    breath_point->is_gap = 1;
                }
                else {
                    // We have not exceeded LOST_COUNTER_MAX times.
                    // Use position from last valid frame. Use value of current frame at this position.
                    // We are progressing blindly (LOST). Stay in tracking mode.
                    tracking = 1;
                    lost_counter++;
                    position = last_position;
                    last_position--;
                    last_value = in_frame[position];

                    breath_point->mode = LOST;
                    breath_point->signal = in_frame[position];
                    breath_point->is_gap = 1;
                }
            }

            // This position is close enough to the previous one, use the new position data
            else {
                tracking = 1;
                lost_counter = 0;
                last_position = position;
                last_value = in_frame[position];

                breath_point->mode = VALID;
                breath_point->signal = in_frame[position];
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

                breath_point->mode = INVALID;
                breath_point->signal = 0.0;
                breath_point->is_lost = 1;
            }
            else {
                // Keep the previous position for at most LOST_COUNTER_MAX times
                // Use position from last valid frame. Use value of current frame at this position.
                // We are progressing blindly (LOST). Stay in tracking mode.
                tracking = 1;
                lost_counter++;
                position = last_position;
                last_value = in_frame[position];

                breath_point->mode = LOST;
                breath_point->signal = in_frame[position];
                breath_point->is_lost = 1;
            }
        }
    }

    breath_point->position = position;
}
*/

// Find frequencies from the breath signal
/*
void  find_frequency(breath_point_t* breath_point, float previous_freq, float sampling_rate, zeroxing_t* zeroxing)
{
    breath_point->is_freq = 0;

    if (breath_point->mode == INVALID) {
        breath_point->frequency = 0.0;
        zeroxing_init(zeroxing);
        breath_point->freq_valid = 0;
    }
    else {
        breath_point->frequency = previous_freq; // keep last frequency found
        int found = zeroxing_put(zeroxing, breath_point->signal);
        if (found) {
            breath_point->freq_valid = 1;
            breath_point->frequency = zeroxing_get(zeroxing, sampling_rate);
            if (previous_freq > 0.0 && fabsf(breath_point->frequency - previous_freq)*60.0 > MAX_BREATH_SLOPE) {
                breath_point->frequency = 0.0;
                zeroxing_init(zeroxing);
                breath_point->is_freq = 1;
                breath_point->freq_valid = 0;
                breath_point->mode = INVALID;

            }
        }
    }
}
*/

void  swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

const char* mode_str(int valid) 
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

// char* display_char(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE], int index, breath_point_t* breath_point)
// {
//     char c = breath_point->position == index ? '|' : ' ';

//     int x = round(in[index] * 256 / DISPLAY_RANGE) + 128;
//     x = x >  255 ? 255 : x;
//     x = x <    0 ?   0 : x;

//     sprintf(out, "\e[94m\e[48;2;%d;%d;%dm%c\e[0m", x, x, x, c);
//     return out;
// }


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

void send_frequency_to_MaxMSP(struct sockaddr_in* server, float frequency)
{
    int len;
    char buffer[100];
    char buffer2[100];

    sprintf(buffer2, "%0.4f", frequency);
    len = tosc_writeMessage(buffer, sizeof(buffer), "slmx4", "s", buffer2);
    // tosc_printOscBuffer(buffer, len);
    sendto(fd, buffer, len, MSG_CONFIRM, (const struct sockaddr *) server, sizeof(sockaddr_in));
}

void send_valid_to_MaxMSP(struct sockaddr_in* server, int valid) {
    int len;
    char buffer[100];
    const char* valid_s = valid ? "T" : "F";

    len = tosc_writeMessage(buffer, sizeof(buffer), "", valid_s);
    // tosc_printOscBuffer(buffer, len);
    sendto(fd, buffer, len, MSG_CONFIRM, (const struct sockaddr *) server, sizeof(sockaddr_in));
}

