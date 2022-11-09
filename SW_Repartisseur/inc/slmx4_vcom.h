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

#define TIMEOUT_MS 1000
#define PORT_NAME_LENGTH 100

class slmx4
{
	serialib serial;

	int  initDevice();
	int  initSerial();
	int  openRadar();
	void closeRadar();
	int  checkACK();

//	char host_ip[128];
	char serialport[PORT_NAME_LENGTH+1];

public:
	int numSamplers;
	enum cmds{rx_wait,frame_start,frame_end,ddc_en, pps};

	slmx4();
	int  begin(const char* port);
	void end();
//	void setHost(const char *addr);
	int  getFrameNormalized(_Float32* frame);
	int  getFrameRaw(_Float32* frame);
	void updateNumberOfSamplers();
	int  iterations();
	int  setRegister(int);

	void flushSerial();
	void closeSerial();
};


#endif
