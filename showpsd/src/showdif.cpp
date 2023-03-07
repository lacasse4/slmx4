#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "slmx4_vcom.h"
#include "peak.h"
#include "serialib.h"

#define MAX_FRAME_SIZE 188
#define NUM_FRAMES 200
#define WINDOW_SIZE 5

#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

FILE* fd = NULL;
slmx4 sensor;
timeOut timer;
float acquisition_frame[MAX_FRAME_SIZE*2];
float raw_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE];
float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE];
unsigned long int times[NUM_FRAMES];

void  difference(float diff_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index);
void  moving_average(float filtered_frames[NUM_FRAMES][MAX_FRAME_SIZE], float in_frames[NUM_FRAMES][MAX_FRAME_SIZE], int num_samples, int frame_index, int window_size);
peak_t find_highest_peak(float* signal, int num_samples, float frame_start, float frame_end, float from, float to);

void display_slmx4_status();
void clean_up(int sig);
void launch_viewer();


int main(int argc, char* argv[])
{
	int i, j;
	// peak_t peak;

	unlink(DATA_FILE_NAME);
	// if (argc == 1) {
	// 	mkfifo(DATA_FILE_NAME, FIFO_MODE);
	// 	launch_viewer();
	// }

	signal(SIGINT, clean_up);

	memset(acquisition_frame, 0, MAX_FRAME_SIZE*2*sizeof(float));
	memset(raw_frames, 0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(diff_frames, 0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));
	memset(filtered_frames, 0, NUM_FRAMES*MAX_FRAME_SIZE*sizeof(float));

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
	printf("Data aquisition starts\n");
	timer.initTimer();
	for (i = 0; i < NUM_FRAMES; i++) {
		sensor.get_frame_normalized(acquisition_frame, POWER_IN_WATT);
		memcpy(raw_frames[i], acquisition_frame, sensor.get_num_samples()*sizeof(float));
		if (i >= 1) {
			difference(diff_frames, raw_frames, sensor.get_num_samples(), i);
		}
		if (i >= WINDOW_SIZE-1) {
			moving_average(filtered_frames, diff_frames, sensor.get_num_samples(), i, WINDOW_SIZE);
		}
		times[i] = timer.elapsedTime_ms();
	}
	printf("Data scquisition ends\n");


	// Write data to file
	printf("Output data to file DATA\n");
	fd = fopen(DATA_FILE_NAME, "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open %s for writing\n", DATA_FILE_NAME);
		exit(EXIT_FAILURE);
	}	

	for (i = 0; i < NUM_FRAMES; i++) {
		fprintf(fd, "%d\n", sensor.get_num_samples());
		fprintf(fd, "%f\n", sensor.get_frame_start());
		fprintf(fd, "%f\n", sensor.get_frame_end());
		for (j = 0; j < sensor.get_num_samples(); j++) {
			fprintf(fd,"%f\n",filtered_frames[i][j]);
		}	
	}

	float average = 0.0;
	unsigned long int last_value = 0L;
	for (i = 0; i < NUM_FRAMES; i++) {
		average += times[i] - last_value;
		last_value = times[i];
	}
	printf("Average acquisiton time = %f ms\n", average/NUM_FRAMES);

	clean_up(0);

	return EXIT_SUCCESS;
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
	if (fd != NULL) fclose(fd);
	fd = NULL;
	sensor.end();
	// unlink(DATA_FILE_NAME);
	exit(EXIT_SUCCESS);
}

void launch_viewer()
{
	int pid = fork();
	if (pid == 0) {
		execlp("python", "python", "python/plotdiff.py", NULL);
		fprintf(stderr, "ERROR - Unable to start viewer\n");
		exit(EXIT_FAILURE);
	}
}

peak_t find_highest_peak_slmx4(slmx4* sensor, float* signal, float from, float to)
{
	return find_highest_peak(signal,
	    sensor->get_num_samples(),
		sensor->get_frame_start(),
		sensor->get_frame_end(),
		from, to);
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
