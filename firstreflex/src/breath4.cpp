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
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/select.h>


#include "slmx4_vcom.h"
#include "serialib.h"
#include "breathFilter.h"
#include "tinyosc.h"
#include "peak.h"

#define MAX_FRAME_SIZE      188
#define START_PEAK_SEARCH   0.45  // in meter (set empiricaly to clear initial radar impulsion)
#define STOP_PEAK_SEARCH    2.00  // in meter
    
#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

slmx4 sensor;
timeOut timer;
float frame[MAX_FRAME_SIZE*2];

char  display[MAX_FRAME_SIZE+1];

float previous_freq;
unsigned long int times;
float sampling_rate;
struct sockaddr_in server_addr;
int   fd;

breathFilter    _filter_br_mem;
breathFilter*    filter_br;
peak_t           peak;

void  apply_breath_filter(float* out_signal, float in_signal, breathFilter* filter);

void  display_slmx4_status(FILE* f);
char* display_signal(char out[MAX_FRAME_SIZE], float in[MAX_FRAME_SIZE]);

void  clean_up(int sig);
void  launch_viewer();
void  swap(int* a, int* b);

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

    previous_freq = 0.0;

	// Use static memory for the various data structures
    // and initialize memory

    filter_br = &_filter_br_mem;
    breathFilter_init(filter_br);

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

    if (sensor.get_num_samples() > MAX_FRAME_SIZE) {
        fprintf(stderr, "Buffer size error\n");
        clean_up(0);
    }

    if (sensor.get_frame_start() > 0.0) {
        fprintf(stderr, "Frame must start at position 0.0\n");
        clean_up(0);
    }

    float samples_per_unit = sensor.get_num_samples() / sensor.get_frame_end();

	// Acquire data
	fprintf(stderr, "Breath frequency detection in progress. Type CTRL_C to exit.\n");

    while(1) {
    	timer.initTimer();
		sensor.get_frame_normalized(frame, POWER_IN_WATT);
		sampling_rate = 1000.0 / timer.elapsedTime_ms();

        peak = find_peak_with_unit(frame, sensor.get_num_samples(), START_PEAK_SEARCH, STOP_PEAK_SEARCH, samples_per_unit);

        if (debug) {
            if (peak.position == -1) fprintf(stderr, "E");
            printf("%7.4f\n", peak.precise_position);

        }
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

// Apply a filter to the breath signal
void apply_breath_filter(float* out_signal, float in_signal, breathFilter* filter)
{
    breathFilter_put(filter, in_signal);
    *out_signal = breathFilter_get(filter);
}


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

