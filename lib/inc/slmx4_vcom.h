#ifndef SLMX4_VCOM_H
#define SLMX4_VCOM_H

#include "serialib.h"

#define BAUD_RATE         115200
#define TIMEOUT_MS        1000
#define ACK_LEN           5         // "<ACK>" string length without '\0'

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

#define ZERO_DB_POWER     530.0
#define POWER_IN_DB		  1
#define POWER_IN_WATT     0

class slmx4
{
	serialib serial;
	char response[MAX_RESPONSE_LEN+1];

	int   refresh_required;
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

	int   init_serial(const char* port);
	void  flush_serial();
	void  close_serial();

	int   init_device();
	int   open_radar();
	int   close_radar();
	int   check_ACK(const char* source);

	int   get_int_value(const char* command);
	float get_float_value(const char* command);
	void  refresh_all_parameters();
	void  ensure_refreshed_data();
	int   get_frame(const char* command, float* frame, int in_db);
	int   get_bytes(const char* comamnd, float* frame, int n_bytes);
	void  compute_psd_in_db(float* frame);
	void  compute_psd(float* frame);
	float compute_power(float re, float im);


public:

	slmx4();
	int   begin(const char* port);
	void  end();

	int   get_frame_normalized(float* frame, int in_db);
	int   get_frame_raw(float* frame, int in_db);

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
	int   is_ddc_en();
	float get_frame_offset();
	float get_frame_start();
	float get_frame_end();
	float get_sweep_time();
	float get_unambiguous_range();
	float get_res();
	float get_fs_rf();

	char* get_value_by_name(const char* command);
	int   set_value_by_name(const char* command);
	
};

#endif