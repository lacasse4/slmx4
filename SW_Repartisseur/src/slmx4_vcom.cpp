/*	Module slmx4_vcom 
 *
 * Auteur: Julian Davis
 *
 * Une traduction en C(++) du script matlab " vcom_xep_radar_connector.m "
 *
 * (github.com/SensorLogicInc/modules/blob/main/matlab/vcom_xep_radar_connector.m)
 *
 */

#include "slmx4_vcom.h"

slmx4::slmx4()
{
	num_samples = 0;
	strcpy(response, "");
}

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

	return SUCCESS;
}

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

int slmx4::init_device()
{
	serial.flushReceiver();
	serial.writeString("NVA_CreateHandle()");
	return check_ACK("init_device()");
}

int slmx4::open_radar()
{
	serial.flushReceiver();
	serial.writeString("OpenRadar(X4)");
	return check_ACK("open_radar()");
}

int slmx4::close_radar()
{
	serial.flushReceiver();
	serial.writeString("Close()");
	return check_ACK("close_device()");
}

int slmx4::get_int_value(const char* command)
{
	char* s;
	s = get_value_by_name(command);
	return atoi(s);
}

float slmx4::get_float_value(const char* command)
{
	char* s;
	s = get_value_by_name(command);
	return atof(s);
}

int slmx4::get_dac_min()
{
	dac_min = get_int_value("VarGetValue_ByName(dac_min)");
	return dac_min;
}

int slmx4::get_dac_max()
{
	dac_max = get_int_value("VarGetValue_ByName(dac_max)");
	return dac_max;
}

int slmx4::get_dac_step()
{
	dac_step = get_int_value("VarGetValue_ByName(dac_step)");
	return dac_step;
}

int slmx4::get_pps()
{
	pps = get_int_value("VarGetValue_ByName(pps)");
	return pps;
}

int slmx4::get_iterations()
{
	iterations = get_int_value("VarGetValue_ByName(iterations)");
	return iterations;
}

int slmx4::get_prf_div()
{
	prf_div = get_int_value("VarGetValue_ByName(prf_div)");
	return prf_div;
}

float slmx4::get_prf()
{
	prf = get_float_value("VarGetValue_ByName(prf)");
	return prf;
}

float slmx4::get_fs()
{
	fs = get_float_value("VarGetValue_ByName(fs)");
	return fs;
}

int slmx4::get_num_samples()
{
	num_samples = get_int_value("VarGetValue_ByName(num_samples)");
	return num_samples;
}

int slmx4::get_frame_length()
{
	frame_length = get_int_value("VarGetValue_ByName(frame_length)");
	return frame_length;
}

int slmx4::get_rx_wait()
{
	rx_wait = get_int_value("VarGetValue_ByName(rx_wait)");
	return rx_wait;
}

int slmx4::get_tx_region()
{
	tx_region = get_int_value("VarGetValue_ByName(tx_region)");
	return tx_region;
}

int slmx4::get_tx_power()
{
	tx_power = get_int_value("VarGetValue_ByName(tx_power)");
	return tx_power;
}

int slmx4::get_ddc_en()
{
	ddc_en = get_int_value("VarGetValue_ByName(ddc_en)");
	return ddc_en;
}

float slmx4::get_frame_offset()
{
	frame_offset = get_float_value("VarGetValue_ByName(frame_offset)");
	return frame_offset;
	
}

float slmx4::get_frame_start()
{
	frame_start = get_float_value("VarGetValue_ByName(frame_start)");
	return frame_start;
}

float slmx4::get_frame_end()
{
	frame_end = get_float_value("VarGetValue_ByName(frame_end)");
	return frame_end;
}

float slmx4::get_sweep_time()
{
	sweep_time = get_float_value("VarGetValue_ByName(sweep_time)");
	return sweep_time;
}

float slmx4::get_unambiguous_range()
{
	unambiguous_range = get_float_value("VarGetValue_ByName(unambiguous_range)");
	return unambiguous_range;
}

float slmx4::get_res()
{
	res = get_float_value("VarGetValue_ByName(res)");
	return res;
}

float slmx4::get_fs_rf()
{
	fs_rf = get_float_value("VarGetValue_ByName(fs_rf)");
	return fs_rf;
}

void slmx4::refresh_all_parameters()
{
	get_dac_min();
	get_dac_max();
	get_dac_step();
	get_pps();
	get_iterations();
	get_prf_div();
	get_prf();
	get_fs();
	get_num_samples();
	get_frame_length();
	get_rx_wait();
	get_tx_region();
	get_tx_power();
	get_ddc_en();
	get_frame_offset();
	get_frame_start();
	get_frame_end();
	get_sweep_time();
	get_unambiguous_range();
	get_res();
	get_fs_rf();
}

int slmx4::get_frame(const char* command, float* frame) 
{
	timeOut timer;
	int n_bytes_read;
	int n_bytes_to_read = num_samples * 4;

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

int slmx4::get_frame_raw(float* frame) 
{
	return get_frame("GetFrameRaw()", frame);
}

int slmx4::get_frame_normalized(float* frame)  
{
	return get_frame("GetFrameNormalized()", frame);
}

void slmx4::end()
{
	close_radar();
	close_serial();
}

void slmx4::flush_serial() 
{
	serial.flushReceiver();
}

void slmx4::close_serial() 
{
	serial.closeDevice();
}

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


int slmx4::set_value_by_name(const char* command)
{
	serial.flushReceiver();
	serial.writeString(command);
	return check_ACK("set_value_by_name");
}
