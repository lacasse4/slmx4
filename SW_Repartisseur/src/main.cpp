#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>

#include "slmx4_vcom.h"

#define MAX_FRAME_SIZE 1535

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

FILE* fd = NULL;
slmx4 sensor;
float sensor_data[MAX_FRAME_SIZE];

void display_slmx4_status();

void clean_up(int sig) {
	printf("Close sensor\n");
	if (fd != NULL) fclose(fd);
	fd = NULL;
	sensor.end();
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
	int i;
	long frame_count = 1;

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

    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");
	display_slmx4_status();

	sensor.get_num_samples();
	printf("sensor.n_samples = %d\n", sensor.num_samples);

	if (argc == 2) {
		clean_up(0);
	}

	sensor.num_samples = sensor.num_samples * 2;

	printf("Output data to file DATA\n");
	fd = fopen("./DATA", "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open ./DATA for writing\n");
		exit(EXIT_FAILURE);
	}
	
	int done = 0;
	while (!done) {
		printf("Frame %ld\r", frame_count++);
		fflush(stdout);
		sensor.get_frame_normalized(sensor_data);

		// sensor.getFrameNormalized(sensor_data);

		fprintf(fd, "%d\n", sensor.num_samples);
		for (i = 0; i < sensor.num_samples; i++) {
			fprintf(fd,"%f\n",sensor_data[i]);
		}
		fflush(fd);
		usleep(500000);
		done = 1;
	}

	fclose(fd);
	fd = NULL;
	printf("Close sensor\n");
	sensor.end();

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
	printf("ddc_en            = %d\n", sensor.get_ddc_en());
	printf("frame_offset      = %e\n", sensor.get_frame_offset());
	printf("frame_start       = %e\n", sensor.get_frame_start());
	printf("frame_end         = %e\n", sensor.get_frame_end());
	printf("sweep_time        = %e\n", sensor.get_sweep_time());
	printf("unambiguous_range = %e\n", sensor.get_unambiguous_range());
	printf("res               = %e\n", sensor.get_res());
	printf("fs_rf             = %e\n", sensor.get_fs_rf());
}