#ifndef SENSOR_DEF
#define SENSOR_DEF

#include <iostream>
#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <stdint.h>

#include "serialib.h"

//#define DEBUG


#define BAUD_RATE         115200
#define TIMEOUT_MS        1000
#define ACK_LEN           5         // "<ACK>" without '\0'

#define PORT_NAME_LENGTH  100
#define MAX_RESPONSE_LEN  100

#define SUCCESS		      0
#define ERROR_ACK 	     -1
#define ERROR_SERIAL     -2
#define ERROR_PARAM      -3
#define ERROR_INIT       -4
#define ERROR_OPEN       -5
#define ERROR_TIMEOUT    -6
#define ERROR_BYTES_READ -7
#define ERROR_MSG_LEN    -8
#define ERROR_READBYTES  -9
#define ERROR_VALUE      -10

class slmx4
{
	serialib serial;
	char response[MAX_RESPONSE_LEN+1];

	int   init_device();
	int   init_serial(const char* port);
	int   open_radar();
	int   close_radar();
	int   check_ACK(const char* source);
	int   get_int_value(const char* command);
	float get_float_value(const char* command);
	int   get_frame(const char* command, float* frame);
	void  flush_serial();
	void  close_serial();


public:
	int   dac_min;
	int   dac_max;
	int   dac_step;
	int   pps;
	int   iterations;
	int   prf_div;
	float prf;
	float fs;
	int   num_samples;
	int   frame_length;
	int   rx_wait;
	int   tx_region;
	int   tx_power;
	int   ddc_en;
	float frame_offset;
	float frame_start;
	float frame_end;
	float sweep_time;
	float unambiguous_range;
	float res;
	float fs_rf;

	slmx4();
	int   begin(const char* port);
	void  end();

	int   get_frame_normalized(float* frame);
	int   get_frame_raw(float* frame);

	int   get_dac_min();
	int   get_dac_max();
	int   get_dac_step();
	int   get_pps();
	int   get_iterations();
	int   get_prf_div();
	float get_prf();
	float get_fs();
	int   get_num_samples();
	int   get_frame_length();
	int   get_rx_wait();
	int   get_tx_region();
	int   get_tx_power();
	int   get_ddc_en();
	float get_frame_offset();
	float get_frame_start();
	float get_frame_end();
	float get_sweep_time();
	float get_unambiguous_range();
	float get_res();
	float get_fs_rf();
	void  refresh_all_parameters();

	char* get_value_by_name(const char* command);
	int   set_value_by_name(const char* command);
	
};

#endif