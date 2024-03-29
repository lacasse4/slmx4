/*
 * breath4.cpp
 * Find respiration rate continuously wih SLMX4 sensor using first reflection
 * Using FFTW library
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
// #include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/time.h>


#include "slmx4_vcom.h"
#include "serialib.h"
#include "tinyosc.h"
#include "peak.h"
#include "fftwrapper.h"
#include "buffer.h"
#include "timer_us.h"

#define MAX_FRAME_SIZE      188
#define NUM_PROCESSORS      1

// Start and stop limits for first reflexion peak search within radar frame
#define START_FRAME_PEAK    0.63f   // in meter (empiricaly set to avoid initial radar impulsion)
#define STOP_FRAME_PEAK     2.00f   // in meter

// Sampling rate first estimate (number of frames per second)
#define TARGET_SAMPLING_RATE    50.00f // in hertz  
#define TARGET_ELAPSED_TIME_US  (1000000f/TARGET_SAMPLING_RATE) // in microsec.

// Start and stop limits for peak seach within breath signal spectrum
// For following definitions, see: https://www.healthline.com/health/normal-respiratory-rate
//       "Normal respiratory rate in a healthy adult is about 12 to 20 breaths per minute".
// The range was slightly extended due to peak detection algorithm requirements.
#define START_SPECTRUM_PEAK (10.0f/60.0f)  // in hertz
#define STOP_SPECTRUM_PEAK  (22.0f/60.0f)  // in hertz
    
#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"
#define SERIAL_PORT "/dev/ttyACM0"

int   fd = 0;
slmx4 sensor;
struct sockaddr_in server_addr;
FILE* fd_debug = NULL;

// buffer_size is set according to the sampling rate and
// the lowest respiration rate we want to detect.
// The lowest respiration rate = 10 RPM -> T = 6 sec.
// The sampling rate is 50 Hz (iterations = 64, pps = 16) 
// number_of_samples = 2 * 6 sec * 50 Hz = 600 samples.
int             buffer_size[NUM_PROCESSORS] = {600};
// int             buffer_size[NUM_PROCESSORS] = {600, 1200, 2400};
float           frame[MAX_FRAME_SIZE*2] = {0};
float           frame_mem[1024][MAX_FRAME_SIZE];
float*          spectrum[NUM_PROCESSORS] = {0};
// peak_t          frame_peak;
peak_t          spectrum_peak[NUM_PROCESSORS];
fft_wrapper_t*  fft_wrapper[NUM_PROCESSORS] = {0};
buffer_t*       breath_signal[NUM_PROCESSORS] = {0};

float sampling_rate;
float samples_per_meter;
float samples_per_hertz[NUM_PROCESSORS];
float previous_freq;
unsigned long int elapsed_time_us;
struct timeval start_time;
float raising_edge;

void  display_slmx4_status(FILE* f);
char* display_signal(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE]);
void  clean_up(int sig);
void  launch_viewer();
void  swap(int* a, int* b);
void  send_frequency_to_MaxMSP(struct sockaddr_in* server, float frequency);
void  send_valid_to_MaxMSP(struct sockaddr_in* server, int valid);
int   write_signal(const char* filename, float* signal, int n);
int   write_img_file(const char* filename, float frames[1024][MAX_FRAME_SIZE]);
void  print_debug(FILE* fd);


int main(int argc, char* argv[])
{
    if (argc != 1 && argc != 3) {
        printf("Usage:\n");
        printf("       breath4 <ip_address> <port>  (with MaxMSP)\n");
        printf(" or    breath4                      (in debug mode)\n");
        return 1;
    }

    int debug = argc == 1;

    if (!debug) {
        struct in_addr addr_binary;
        if (inet_pton(AF_INET, argv[1], &addr_binary) == 0) {
            fprintf(stderr, "Error: invalid ip address\n");
            exit(1);
        };

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1) {
            fprintf(stderr, "Error: unable to open UDP connection\n");
            exit(1);
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr = addr_binary;
        server_addr.sin_port = htons(atoi(argv[2]));
    }

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

	// from now on, clean_up(0) is the proper way to exit this program
    // clean_up() will be executed if CTRL_C is entered
	signal(SIGINT, clean_up);

	// Set sensor operating mode, that is:
	//   - 64 iterations
	//   - 16 pulses per step (pps)
	//   - dac minimum value 896   (1024 - 128)
	//   - dac maximum value 1152  (1024 + 128)
	//   - downconversion and decimation enabled (ddc = 1)
	// The 4 first values were experimentaly set.
	// The iteration and pps values yield a smooth signal (less noise)
	// but increase the data acquisition time between frames.
	// By increasing dac_min and reducing dac_max, the data acquisition
	// time is reduced. This application does not require a larger range.
	// With this setup, the acquition rate is about 1 frame each 20 ms,
	// or 50.0 frames per second. 
 	//
	// Setting ddc to 1 enable the downconverion and decimation algorithm 
	// available on the SLMX4 sensor. It demodulates the radar signal 
	// from its carrier frequency. See:
	// https://sensorlogicinc.github.io/modules/docs/XTAN-13_XeThruX4RadarUserGuide_rev_a.pdf section 5.
	// The result is 188 points in complex numbers (float*2). 
	// Each complex data point is converted to its modulus in get_frame_normalized().  
    // Each data point correspond to a distance of 0.0525 m (5.25 cm).

    sensor.set_value_by_name("VarSetValue_ByName(iterations,64)");
    sensor.set_value_by_name("VarSetValue_ByName(pps,16)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_min,896)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_max,1152)");
    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");
	display_slmx4_status(stderr);

    if (sensor.get_num_samples() > MAX_FRAME_SIZE) {
        fprintf(stderr, "Buffer size error\n");
        clean_up(0);
    }

    if (sensor.get_frame_start() > 0.0) {
        fprintf(stderr, "Frame must start at position 0.0\n");
        clean_up(0);
    }

    // Allocate data structure and initialize memory
    for (int i = 0; i < NUM_PROCESSORS; i++) {
        breath_signal[i] = buffer_init(buffer_size[i]);
        if (breath_signal[i] == NULL) {
            clean_up(0);
        }
        spectrum[i] = (float*)malloc(sizeof(float)*buffer_size[i]);
        if (spectrum[i] == NULL) {
            clean_up(0);
        }
        fft_wrapper[i] = fft_init(buffer_size[i]);
        if (fft_wrapper[i] == NULL) {
            clean_up(0);
        }
        samples_per_hertz[i] = buffer_size[i] / TARGET_SAMPLING_RATE;
    }

    previous_freq = 0.0;
    samples_per_meter = sensor.get_num_samples() / sensor.get_frame_end();

	// Acquire data
	fprintf(stderr, "Breath frequency detection in progress. Type CTRL_C to exit.\n");

    fd_debug = fopen("SCREEN", "w");

    int k = 0;
    while(k < 1024) {
    // while(1) {
        start_time = timer_us_init();
		sensor.get_frame_normalized(frame, POWER_IN_WATT);

        memcpy(frame_mem[k], frame, sizeof(float)*MAX_FRAME_SIZE);
        k++;

        raising_edge = find_peak_raising_edge_with_unit (frame, sensor.get_num_samples(), START_FRAME_PEAK, STOP_FRAME_PEAK, samples_per_meter);    
        for (int i = 0; i < NUM_PROCESSORS; i++) {
            buffer_put_sample(breath_signal[i], raising_edge);
            fft_spectrum(fft_wrapper[i], buffer_get_buffer(breath_signal[i]), spectrum[i]);
            spectrum[i][0] = 0.0; // remove DC component
            spectrum_peak[i] = find_peak_with_unit(spectrum[i], buffer_size[i]/2+1, START_SPECTRUM_PEAK, STOP_SPECTRUM_PEAK, samples_per_hertz[i]);
        }
        if (raising_edge == -1.0) {
            for(int i = 0; i < NUM_PROCESSORS; i++) {
                buffer_reset(breath_signal[i]);
            }
        }

   		elapsed_time_us = timer_us_elapsed(start_time);
        sampling_rate = 1000000.0f / elapsed_time_us;

        print_debug(stdout);
        print_debug(fd_debug);
    }

    write_img_file("FRAMES", frame_mem);

	clean_up(0);
	return EXIT_SUCCESS;
}

void print_debug(FILE* fd) 
{
    fprintf(fd, "%0.4f ", raising_edge);
    float resp = spectrum_peak[0].scaled_position*60.0;
    if (!buffer_is_valid(breath_signal[0])) resp = 0.0;
    fprintf(fd, "%0.4f ", resp);
    fprintf(fd, "\n");
    // fprintf(fd, "\r");
    // fflush(fd);



    // fprintf(fd, "%0.4f ", sampling_rate);
    // fprintf(fd, "%2d ",   frame_peak.position); 
    // fprintf(fd, "%0.4f ", frame_peak.precise_position);
    // fprintf(fd, "%0.2f ", raising_edge);
    // fprintf(fd, "%d ",    buffer_is_valid(breath_signal[0]));
    // fprintf(fd, "%3d ",   buffer_get_counter(breath_signal[0]));
    // fprintf(fd, "%0.4f ", spectrum_peak[0].scaled_position*60.0);
    // fprintf(fd, "%2d ",   spectrum_peak[0].position);
    // fprintf(fd, "%0.4f ", spectrum_peak[0].precise_position);
    // fprintf(fd, "%0.4f ", spectrum_peak[0].scaled_position*60.0);
    // fprintf(fd, "%0.4f ", spectrum_peak[0].precise_value);
        // printf("%d ",    buffer_is_valid(breath_signal[1]));
        // printf("%3d ",   buffer_get_counter(breath_signal[1]));
        // printf("%2d ",   spectrum_peak[1].position);
        // printf("%7.4f ", spectrum_peak[1].precise_position*60.0);
        // printf("%7.4f | ", spectrum_peak[1].precise_value);
        // printf("%d ",    buffer_is_valid(breath_signal[2]));
        // printf("%3d ",   buffer_get_counter(breath_signal[2]));
        // printf("%2d ",   spectrum_peak[2].position);
        // printf("%7.4f ", spectrum_peak[2].precise_position*60.0);
        // printf("%7.4f",  spectrum_peak[2].precise_value);
    // fprintf(fd, "\n");

            // if (buffer_is_valid(breath_signal[0]) && spectrum_peak[0].position == -1) {
            //     buffer_dump(breath_signal[0], "SIGNAL");
            //     write_signal("SPECTRUM", spectrum[0], buffer_size[0]/2+1);
            //     clean_up(0);
            // }
            // if (first_frame_peak.position == -1) {
            //     write_signal("FRAME", frame, MAX_FRAME_SIZE);
            //     clean_up(0);
            // }

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
	fprintf(stderr, "\nTerminated normally\n");
	sensor.end();
    if (fd != 0) {
        close(fd);
    }
    if (fd_debug != NULL) {
        fclose(fd_debug);
    }

	// unlink(DATA_FILE_NAME);

    for (int i = 0; i < NUM_PROCESSORS; i++) {
        buffer_release(breath_signal[i]);
        free(spectrum[i]);
        fft_release(fft_wrapper[i]);
    } 

    fft_cleanup();

    if (sig == SIGINT) {
    	exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
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

// Apply a filter to the breath signal
// void apply_breath_filter(float* out_signal, float in_signal, breathFilter* filter)
// {
//     breathFilter_put(filter, in_signal);
//     *out_signal = breathFilter_get(filter);
// }


void  swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
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

int write_signal(const char* filename, float* signal, int n)
{
	FILE* fd = fopen(filename, "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open %s for writing\n", filename);
		return 1;
	}	

	for (int i = 0; i < n; i++) {
		fprintf(fd, "%7.4f\n", signal[i]);
	}

	fclose(fd);
	return 0;
}


// Write of text file containing NUM_FRAMES frames
int write_img_file(const char* filename, float frames[1024][MAX_FRAME_SIZE])
{
	// Write data to file
	FILE* fd = fopen(filename, "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open %s for writing\n", filename);
		return 1;
	}	

	for (int i = 0; i < 1024; i++) {
		for (int j = 0; j < MAX_FRAME_SIZE; j++) {
			fprintf(fd,"%f\n", frames[i][j]);
		}	
	}

	fclose(fd);
	return 0;
}
