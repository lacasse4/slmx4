/* Module slmx4_vcom.cpp 
 * Auteur: Vincent Lacasse
 *
 * driver slmx4 utilisant le 'XEP Matlab Connector'
 * ref: https://sensorlogicinc.github.io/modules/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "slmx4_vcom.h"

// ------------------------------------------------
//               Public methods
// ------------------------------------------------

// Constructor
slmx4::slmx4()
{
	strcpy(response, "");	
	refresh_required = 1;
	dac_min = 0;
	dac_max = 0;
	dac_step = 0;
	pps = 0;
	iterations = 0;
	prf_div = 0;
	prf = 0.0;
	fs = 0.0;
	num_samples = 0;
	frame_length = 0;
	rx_wait = 0;
	tx_region = 0;
	tx_power = 0;;
	ddc_en = 0;
	frame_offset = 0.0;
	frame_start = 0.0;
	frame_end = 0.0;
	sweep_time = 0.0;
	unambiguous_range = 0.0;
	res = 0.0;
	fs_rf = 0.0;
}

// start a session with a SLMX4
// this should be the first method called
int slmx4::begin(const char *port)
{
	if (init_serial(port) != SUCCESS) {
		return ERROR_SERIAL;
	}

	if (init_device() != SUCCESS) {
		close_serial();
		return ERROR_INIT;
	}

	if (open_radar() != SUCCESS) {
		close_radar();
		return ERROR_OPEN;
	}

    refresh_required = 1;
	return SUCCESS;
}

// end a session with a SLMX4
// this must be called at the end of a session for proper clean up
void slmx4::end()
{
	close_radar();
	close_serial();
}

// ------- getters ---------

int slmx4::get_dac_min()
{
	ensure_refreshed_data();
	return dac_min;
}

int slmx4::get_dac_max()
{
	ensure_refreshed_data();
	return dac_max;
}

int slmx4::get_dac_step()
{
	ensure_refreshed_data();
	return dac_step;
}

int slmx4::get_pps()
{
	ensure_refreshed_data();
	return pps;
}

int slmx4::get_iterations()
{
	ensure_refreshed_data();
	return iterations;
}

int slmx4::get_prf_div()
{
	ensure_refreshed_data();
	return prf_div;
}

float slmx4::get_prf()
{
	ensure_refreshed_data();
	return prf;
}

float slmx4::get_fs()
{
	ensure_refreshed_data();
	return fs;
}

int slmx4::get_num_samples()
{
	ensure_refreshed_data();
	return num_samples;
}

int slmx4::get_frame_length()
{
	ensure_refreshed_data();
	return frame_length;
}

int slmx4::get_rx_wait()
{
	ensure_refreshed_data();
	return rx_wait;
}

int slmx4::get_tx_region()
{
	ensure_refreshed_data();
	return tx_region;
}

int slmx4::get_tx_power()
{
	ensure_refreshed_data();
	return tx_power;
}

int slmx4::is_ddc_en()
{
	ensure_refreshed_data();
	return ddc_en;
}

float slmx4::get_frame_offset()
{
	ensure_refreshed_data();
	return frame_offset;
}

float slmx4::get_frame_start()
{
	ensure_refreshed_data();
	return frame_start;
}

float slmx4::get_frame_end()
{
	ensure_refreshed_data();
	return frame_end;
}

float slmx4::get_sweep_time()
{
	ensure_refreshed_data();
	return sweep_time;
}

float slmx4::get_unambiguous_range()
{
	ensure_refreshed_data();
	return unambiguous_range;
}

float slmx4::get_res()
{
	ensure_refreshed_data();
	return res;
}

float slmx4::get_fs_rf()
{
	ensure_refreshed_data();
	return fs_rf;
}

// Get a radar frame in raw format
int slmx4::get_frame_raw(float* frame, int in_db) 
{
	return get_frame("GetFrameRaw()", frame, in_db);
}

// Get a radar frame in normalized format
int slmx4::get_frame_normalized(float* frame, int in_db)  
{
	return get_frame("GetFrameNormalized()", frame, in_db);
}

// Send a command such as "VarGetValue_ByName(dac_min)"
// to SLMX4 and receive data as a string.  
// Object parameters are not modified by this method. For debugging only.
// The use of getters is saffer.
char* slmx4::get_value_by_name(const char* command)
{
	int n_bytes_read;

	serial.flushReceiver();
	serial.writeString(command);
	n_bytes_read = serial.readString(response, '>', MAX_RESPONSE_LEN, TIMEOUT_MS);

	// strip out "<ACK>" and terminate string with NULL char
	if (n_bytes_read >= 5) 
		response[n_bytes_read-5] = '\0';
	
	return response;
}

// Send a command such as "VarSetValue_ByName(dac_min)"
// Such command modifies SLMX4 internal parameters
int slmx4::set_value_by_name(const char* command)
{
	serial.flushReceiver();
	serial.writeString(command);
    refresh_required = 1;
	return check_ACK("set_value_by_name");
}


// ------------------------------------------------
//               Private methods
// ------------------------------------------------

// Initialize serial communication with SLMX4
int slmx4::init_serial(const char* port)
{
	// Connection to serial port
	if (serial.openDevice(port, BAUD_RATE) != 1) {
		return ERROR_SERIAL;
	}

	serial.setDTR();
	serial.setRTS();
	serial.flushReceiver();
	return SUCCESS;
}

// Check if ACK message has been properly received
// ACK message is received after most (but not all) commands
// sent to the SLMX4
int slmx4::check_ACK(const char* source)
{
	char buffer[MAX_RESPONSE_LEN+1];
	
	int nBytes = serial.readBytes(buffer, MAX_RESPONSE_LEN, TIMEOUT_MS);
	serial.flushReceiver();

	if (nBytes >= 5 && strncmp(buffer, "<ACK>", nBytes) == 0) { 
		return SUCCESS; 
	}
	else {
		fprintf(stderr, "ERROR - ACK expected and not received in %s\n", source);
		return ERROR_ACK;
	}
}

// Initialize SLMX4
int slmx4::init_device()
{
	serial.flushReceiver();
	serial.writeString("NVA_CreateHandle()");
 	return check_ACK("init_device()");
}

// Start radar functions and set internal parameter to defaults
/*
	Defaults parameters:
	VarGetValue_ByName(dac_min)           = 0
	VarGetValue_ByName(dac_max)           = 2047
	VarGetValue_ByName(dac_step)          = 0
	VarGetValue_ByName(pps)               = 10
	VarGetValue_ByName(iterations)        = 8
	VarGetValue_ByName(prf_div)           = 16
	VarGetValue_ByName(prf)               = 1.518750e+07
	VarGetValue_ByName(fs)                = 2.332800e+10
	VarGetValue_ByName(num_samples)       = 1535
	VarGetValue_ByName(frame_length)      = 16
	VarGetValue_ByName(rx_wait)           = 1
	VarGetValue_ByName(tx_region)         = 3
	VarGetValue_ByName(tx_power)          = 2
	VarGetValue_ByName(ddc_en)            = 0
	VarGetValue_ByName(frame_offset)      = 0.000000e+00
	VarGetValue_ByName(frame_start)       = 0.000000e+00
	VarGetValue_ByName(frame_end)         = 9.870113e+00
	VarGetValue_ByName(sweep_time)        = 1.078255e-02
	VarGetValue_ByName(unambiguous_range) = 9.869711e+00
	VarGetValue_ByName(res)               = 6.430041e-03
	VarGetValue_ByName(fs_rf)             = 2.332800e+10
*/
int slmx4::open_radar()
{
	serial.flushReceiver();
	serial.writeString("OpenRadar(X4)");
    refresh_required = 1;
	return check_ACK("open_radar()");
}

// Stop radar
int slmx4::close_radar()
{
	serial.flushReceiver();
	serial.writeString("Close()");
	return check_ACK("close_device()");
}

// Flush serial input buffer
void slmx4::flush_serial() 
{
	serial.flushReceiver();
}

// Close the serial connection with SLMX4
void slmx4::close_serial() 
{
	serial.closeDevice();
}

// Send a command to SLMX4, receive data and decaode as a int value
int slmx4::get_int_value(const char* command)
{
	char* s;
	s = get_value_by_name(command);
	return atoi(s);
}

// Send a command to SLMX4, receive data and decaode as a float value
float slmx4::get_float_value(const char* command)
{
	char* s;
	s = get_value_by_name(command);
	return atof(s);
}

// Refresh all object parameters if required"VarGetValue_ByName(dac_min)"
// All parameters will be download from SLMX4.
// Any parameter may change on the SLMX4 after command
// such as "OpenRadar(X4)" or "VarSetValue_ByName()"
void slmx4::ensure_refreshed_data()
{
	if (refresh_required) {
		refresh_all_parameters();
		refresh_required = 0;
	}
}

// Download all SLMX4 parameters to this object
void slmx4::refresh_all_parameters()
{
	dac_min           = get_int_value("VarGetValue_ByName(dac_min)");
	dac_max           = get_int_value("VarGetValue_ByName(dac_max)");
	dac_step          = get_int_value("VarGetValue_ByName(dac_step)");
	pps               = get_int_value("VarGetValue_ByName(pps)");
	iterations        = get_int_value("VarGetValue_ByName(iterations)");
	prf_div           = get_int_value("VarGetValue_ByName(prf_div)");
	prf               = get_float_value("VarGetValue_ByName(prf)");
	fs                = get_float_value("VarGetValue_ByName(fs)");
	num_samples       = get_int_value("VarGetValue_ByName(num_samples)");
	frame_length      = get_int_value("VarGetValue_ByName(frame_length)");
	rx_wait           = get_int_value("VarGetValue_ByName(rx_wait)");
	tx_region         = get_int_value("VarGetValue_ByName(tx_region)");
	tx_power          = get_int_value("VarGetValue_ByName(tx_power)");
	ddc_en            = get_int_value("VarGetValue_ByName(ddc_en)");
	frame_offset      = get_float_value("VarGetValue_ByName(frame_offset)");
	frame_start       = get_float_value("VarGetValue_ByName(frame_start)");
	frame_end         = get_float_value("VarGetValue_ByName(frame_end)");
	sweep_time        = get_float_value("VarGetValue_ByName(sweep_time)");
	unambiguous_range = get_float_value("VarGetValue_ByName(unambiguous_range)");
	res               = get_float_value("VarGetValue_ByName(res)");
	fs_rf             = get_float_value("VarGetValue_ByName(fs_rf)");
}

// Get a radar frame from SLMX4
// Compute power spectrum density if required
int slmx4::get_frame(const char* command, float* frame, int in_db) 
{
	int n_bytes;
	int status;

	if (is_ddc_en()) {
		// interleaved complex data is received
		n_bytes = num_samples * sizeof(float) * 2;
		status = get_bytes(command, frame, n_bytes);
		if (status != SUCCESS) {
			return status;
		}
		if (in_db) {
			compute_psd_in_db(frame);
		}
		else {
			compute_psd(frame);
		}
	}
	else {
		n_bytes = num_samples * sizeof(float);
		return get_bytes(command, frame, n_bytes);
	}

	return SUCCESS;
}

// Get a number of bytes from SLMX4 and place them in 'frame'
int slmx4::get_bytes(const char* command, float* frame, int n_bytes)
{
	timeOut timer;
	int n_bytes_read;
	int n_bytes_to_read = n_bytes;

	serial.flushReceiver();
	serial.writeString(command);
	
	timer.initTimer();
	while (serial.available() == 0) {
		if (timer.elapsedTime_ms() >= TIMEOUT_MS) {
			return ERROR_TIMEOUT;
		}
	}

	n_bytes_read = serial.readBytes(frame, n_bytes_to_read, 1);
	serial.flushReceiver();
	if (n_bytes_read != n_bytes_to_read) {
		return ERROR_BYTES_READ;
	}

	return SUCCESS;
}

/*
 * Compute the power spectrum density (psd) in db 
 * frame provided that:
 *   - its size is num_samples * 2
 * 	 - real and imaginary data are interleaved
 * The result is half the size of the input data and
 * it is place back in 'frame'.
 * The second half of 'frame' is set to 0.0
 */ 
void slmx4::compute_psd_in_db(float* frame)
{
	int i;
	float re, im, power;

	// re = frame[0];
	// im = frame[1];
	// float zero_db = compute_power(re, im);

	for (i = 0; i < num_samples; i++) {
		re = frame[i*2];
		im = frame[i*2+1];
		power = compute_power(re, im);
		frame[i] = 10*log10f(power/ZERO_DB_POWER);
		// frame[i] = 10*log10f(power/zero_db);
	}

	memset(&frame[num_samples], 0, num_samples*4);
}

/*
 * Compute the power spectrum density (psd)
 * frame provided that:
 *   - its size is num_samples * 2
 * 	 - real and imaginary data are interleaved
 * The result is half the size of the input data and
 * it is place back in 'frame'.
 * The second half of 'frame' is set to 0.0
 */ 
void slmx4::compute_psd(float* frame)
{
	int i;
	float re, im;

	for (i = 0; i < num_samples; i++) {
		re = frame[i*2];
		im = frame[i*2+1];
		frame[i] = sqrt(compute_power(re, im));
	}

	memset(&frame[num_samples], 0, num_samples*4);
}


float slmx4::compute_power(float re, float im)
{
	return powf(re, 2) + powf(im, 2);
}
