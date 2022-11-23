/*
 * Module: slm4x_driver.c
 * Auteur: Vincent Lacasse
 * Date: 2022-11-20
 * 
 * SLM4X driver interfacing with SensorLogic vcom_xep_matlab_server
 */

#ifndef _SLM4X_DRIVER_H
#define _SLM4X_DRIVER_H

#define SLM4X_SUCCESS 0
#define SLM4X_INVALID_PARAM -1
#define SLM4X_SERIAL_FAILURE -2
#define SLM4X_INIT_FAILURE -3
#define SLM4X_OPEN_FAILURE -4


int slm4x_init(const char *port);

class slmx4
{
	serialib serial;
    int n_samples;

	int   initDevice();
	int   initSerial();
	int   openRadar();
	void  closeRadar();
	int   checkACK();

	char response[MAX_RESPONSE_LEN+1];

public:
	enum cmds{rx_wait, frame_start, frame_end, ddc_en, pps};

	slmx4();
	int   init(const char* port);
	void  close();
    int   get_frame_normalized(float* frame);
    int   get_frame_raw(float* frame);
	void  updateNumberOfSamplers();
	int   iterations();
	int   setRegister(int);

	void  flushSerial();
	void  closeSerial();

	char* get_value_by_name(const char* command);
	int getFrameNormalized_debug(_Float32* frame, int* n);
	int getFrameNormalized_debug2(_Float32* frame);

};

#endif
#endif