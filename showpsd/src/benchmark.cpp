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

#define MAX_FRAME_SIZE 1535

const char* SERIAL_PORT = "/dev/ttyACM0";

slmx4 sensor;
float sensor_data[MAX_FRAME_SIZE];


void display_slmx4_status();
void clean_up(int sig);

int main(int argc, char* argv[])
{
	timeOut timer;

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
    sensor.set_value_by_name("VarSetValue_ByName(pps,16)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_min,896)");
    sensor.set_value_by_name("VarSetValue_ByName(dac_max,1152)");
    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");

	display_slmx4_status();
	
	unsigned long int ms = 0L;
	for (int i = 0; i < 1000; i++) {
		timer.initTimer();
		sensor.get_frame_normalized(sensor_data, POWER_IN_WATT);
		ms += timer.elapsedTime_ms();
	}
	printf("\nMean time to aquire a frame = %f ms\n", ms/1000.0);

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
	sensor.end();
	exit(EXIT_SUCCESS);
}