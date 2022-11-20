#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>

#include "slmx4_vcom.h"

// Frame capture macros
#define BREATH_SIZE 300
#define PERIOD 50
#define MAX_TIMEOUTS 3

// Sensor register macros
#define RX_WAIT 1
#define FRAME_START 2
#define FRAME_END 3
#define DDC_EN 4
#define PPS 5


const char* SERIAL_PORT = "/dev/serial/by-id/usb-NXP_SEMICONDUCTORS_MCU_VIRTUAL_COM_DEMO-if00";

FILE* fd;
slmx4 sensor;
_Float32* sensor_data;

void stop(int sig) {
	fclose(fd);
	printf("Close sensor\n");
	sensor.end();
	free(sensor_data);
	exit(EXIT_SUCCESS);
}

int main()
{
	int i;
	long frame_count = 1;

	signal(SIGINT, stop);

	if (access(SERIAL_PORT, F_OK) < 0) {
		fprintf(stderr, "ERROR: serial port NOT available: %s\n", SERIAL_PORT);		
		exit(EXIT_FAILURE);
	}
	printf("Serial port available:  %s\n", SERIAL_PORT);

	printf("Initialize SLMX4 sensor\n");
	if (sensor.begin(SERIAL_PORT) == EXIT_FAILURE) exit(EXIT_FAILURE);
	sensor.iterations();//Default values

	//If communication is bad stahp
	printf("Set SLMX4 registers\n");
	sensor.setRegister(slmx4::rx_wait);
	sensor.setRegister(slmx4::frame_start);
	sensor.setRegister(slmx4::frame_end);
	sensor.setRegister(slmx4::ddc_en);
	sensor.setRegister(slmx4::pps);

	sensor_data = (_Float32*)malloc(sizeof(_Float32)*sensor.numSamplers);

	printf("Output data to file DATA\n");
	fd = fopen("./DATA", "w");
	if(fd == NULL) {
		fprintf(stderr,"ERROR: Can not open ./DATA for writing\n");
		exit(EXIT_FAILURE);
	}
	
	while (1) {
		printf("Frame %ld\r", frame_count++);
		fflush(stdout);

		sensor.getFrameNormalized(sensor_data);

		fprintf(fd, "%d\n", sensor.numSamplers);
		for (i = 0; i < sensor.numSamplers; i++) {
			fprintf(fd,"%f\n",sensor_data[i]);
		}
		fflush(fd);
		usleep(500000);
	}

	fclose(fd);
	printf("Close sensor\n");
	sensor.end();
	free(sensor_data);

	return EXIT_SUCCESS;
}
