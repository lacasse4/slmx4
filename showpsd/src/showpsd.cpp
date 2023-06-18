#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "slmx4_vcom.h"

#define MAX_FRAME_SIZE 1535
#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

FILE* fd = NULL;
slmx4 sensor;
float sensor_data[MAX_FRAME_SIZE];


typedef struct peak {
    int index;           // peak index in signal array
    float distance;      // peak distance
    float power;         // peak power in db
} peak_t;

void   erase_peak(peak_t* peak);
peak_t find_highest_peak(slmx4* sensor, float* signal, float from, float to);
int    distance_to_index(float frame_start, float frame_end, int num_samples, float distance);
float  index_to_distance(float frame_start, float frame_end, int num_samples, float index);

void display_slmx4_status();
void clean_up(int sig);
void launch_viewer();


int main(int argc, char* argv[])
{
	int i;
	long frame_count = 1;
	peak_t peak;

	unlink(DATA_FILE_NAME);
	if (argc == 1) {
		mkfifo(DATA_FILE_NAME, FIFO_MODE);
		launch_viewer();
	}

	signal(SIGINT, clean_up);

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

	printf("Output data to file DATA\n");
	fd = fopen(DATA_FILE_NAME, "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open %s for writing\n", DATA_FILE_NAME);
		exit(EXIT_FAILURE);
	}	
	
	while (1) {
		printf("Frame %ld, ", frame_count++);
		fflush(stdout);
		sensor.get_frame_normalized(sensor_data, POWER_IN_DB); // WARNING - DB are now calculated in reference to the first point of radar data.
		peak = find_highest_peak(&sensor, sensor_data, 0.7, 2.0);
		printf("distance = %5.3f, ", peak.distance);

		fprintf(fd, "%d\n", sensor.get_num_samples());
		fprintf(fd, "%f\n", sensor.get_frame_start());
		fprintf(fd, "%f\n", sensor.get_frame_end());
		for (i = 0; i < sensor.get_num_samples(); i++) {
			fprintf(fd,"%f\n",sensor_data[i]);
		}
		fflush(fd);

		usleep(300000);
		printf("\r");

		if (argc != 1) {
			break;
		}
	}

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
	unlink(DATA_FILE_NAME);
	exit(EXIT_SUCCESS);
}

void launch_viewer()
{
	int pid = fork();
	if (pid == 0) {
		execlp("python", "python", "python/plotgraph.py", NULL);
		fprintf(stderr, "ERROR - Unable to start viewer\n");
		exit(EXIT_FAILURE);
	}
}


peak_t find_highest_peak(slmx4* sensor, float* signal, float from, float to)
{  
	int   low_index;
	int   high_index;
	int   index;
	float power;
	int   is_max;

	float frame_start = sensor->get_frame_start();
	float frame_end   = sensor->get_frame_end();
	int   num_samples = sensor->get_num_samples();

	// to be returned if a peak is not found
	peak_t not_found;
	erase_peak(&not_found);

	// 'from' and 'to' must be within the frame_start -> frame_end range
	low_index = distance_to_index(frame_start, frame_end, num_samples, from);
	if (low_index <= 0) return not_found;

	high_index = distance_to_index(frame_start, frame_end, num_samples, to);
	if (high_index >= num_samples-1) return not_found;

	// find max peak in specified range
	index = low_index;
	power = signal[low_index];
	for (int i = low_index; i <= high_index; i++) {
		if (power < signal[i]) {
			power = signal[i]; 
			index = i;
		}
	}

	// there is no peak if max_index is a boundary value
	if (index == low_index) return not_found;
	if (index == high_index) return not_found;

	// value at max_index must be surrounded by lower values
	is_max = signal[index] > signal[index-1] &&
			 signal[index] > signal[index+1];
	if (!is_max) return not_found;

	// find peak accuratly
	peak_t precise_peak;
  	float delta;
  	float interpolation;

  	delta = (signal[index-1] - signal[index+1]) * 0.5 /
            (signal[index-1] - (2.0 * signal[index]) + signal[index+1]);

  	interpolation = index_to_distance(frame_start, frame_end, num_samples, index + delta);

  	precise_peak.index = index;
  	precise_peak.distance = interpolation;
  	precise_peak.power = (signal[index-1] + (2.0 * signal[index]) + signal[index+1]) / 4;

  	return precise_peak;
}


int distance_to_index(float frame_start, float frame_end, int num_samples, float distance) 
{
  return roundf(num_samples * (distance - frame_start) / (frame_end - frame_start));
}


float index_to_distance(float frame_start, float frame_end, int num_samples, float index) 
{
  return frame_start + index * (frame_end - frame_start) / num_samples;
}


void erase_peak(peak_t* peak) {
    peak->index = -1;
    peak->distance = 0.0;
    peak->power = 0.0;
};