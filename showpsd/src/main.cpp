#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "slmx4_vcom.h"

#define MAX_FRAME_SIZE 1535
#define FIFO_MODE 0666
#define DATA_FILE_NAME "./DATA"

const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

FILE* fd = NULL;
slmx4 sensor;
float sensor_data[MAX_FRAME_SIZE];

void display_slmx4_status();
void clean_up(int sig);
void launch_viewer();


int main(int argc, char* argv[])
{
	int i;
	long frame_count = 1;

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

    sensor.set_value_by_name("VarSetValue_ByName(ddc_en,1)");
	display_slmx4_status();

	printf("Output data to file DATA\n");
	fd = fopen(DATA_FILE_NAME, "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open %s for writing\n", DATA_FILE_NAME);
		exit(EXIT_FAILURE);
	}

	int size = 0;
	int ifd = fileno(fd);
	
	while (1) {
		printf("Frame %ld, ", frame_count++);
		fflush(stdout);
		sensor.get_frame_normalized(sensor_data);

		// sensor.getFrameNormalized(sensor_data);

		fprintf(fd, "%d\n", sensor.get_num_samples());
		for (i = 0; i < sensor.get_num_samples(); i++) {
			fprintf(fd,"%f\n",sensor_data[i]);
		}
		fflush(fd);

		usleep(250000);
		ioctl(ifd, FIONREAD, &size);
		printf(" size = %d      \r", size);

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
	exit(EXIT_SUCCESS);
}

void launch_viewer()
{
	int pid = fork();
	if (pid == 0) {
		execlp("python", "python", "plotgraph.py", NULL);
		fprintf(stderr, "ERROR - Unable to start viewer\n");
		exit(EXIT_FAILURE);
	}
}