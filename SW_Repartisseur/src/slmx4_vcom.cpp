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
//#include "sendosc.h"

#define MAX_BYTES_READ 128
#define ACK_SIZE 6

#define DEBUG


slmx4::slmx4()
{
	numSamplers = 0;
}

int slmx4::begin(const char *port)
{
	timeOut timer;
	timer.initTimer();

	if (strlen(port) > PORT_NAME_LENGTH) return EXIT_FAILURE;
	strcpy(serialport, port);

	if (initSerial() == EXIT_FAILURE) return EXIT_FAILURE;
	printf("slmx4::begin - initSerial() successful\n");

	if (initDevice() == EXIT_FAILURE) return EXIT_FAILURE;
	printf("slmx4::begin - initDevice() successful\n");

	serial.setDTR();
    serial.setRTS();
	serial.flushReceiver();

	if (openRadar() == EXIT_FAILURE) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


int slmx4::initSerial()
{
	// Connection to serial port
	char errorOpening = serial.openDevice(serialport, 115200);

	// Share buffer ptr to serial module
	//serial.set_buffer_ptr(buf_ptr);

    // If connection fails, return the error code otherwise, display a success message
    if (errorOpening != 1) {
    	fprintf(stderr, "ERROR connection to serial port: %i\n", errorOpening);
//    	sendosc(string_, (void*)"ERROR connection to serial port", host_ip);
		return EXIT_FAILURE;
    }

	serial.setDTR();
	serial.setRTS();
	serial.flushReceiver();
	return EXIT_SUCCESS;
}


int slmx4::initDevice()
{
	serial.writeString("NVA_CreateHandle()");

	if (checkACK() == EXIT_SUCCESS) {
		#ifdef DEBUG
 		fprintf(stderr, "SUCCESS: ACK from 'initDevice'\n");
//		sendosc(string_, (void*)"SUCCESS: ACK from 'init_device'", host_ip);
		#endif
	}
 	else { 
		#ifdef DEBUG
		fprintf(stderr, "ERROR: Reading ACK in 'initDevice'\n");
//		sendosc(string_, (void*)"ERROR: Reading ACK in 'init_device'\n", host_ip);
		return EXIT_FAILURE;
		#endif
	}
	return EXIT_SUCCESS;
}


int slmx4::checkACK()
{
	char buffer[MAX_BYTES_READ];
	int i;
	
	int nBytes = serial.readBytes(buffer, MAX_BYTES_READ-1, TIMEOUT_MS);
	serial.flushReceiver();

	printf("slmx4::checkACK() - nBytes = %d\n", nBytes);
	printf("slmx4::checkACK() - buffer = ");
	for (i = 0; i <nBytes; i++) printf("%c", buffer[i]);
	printf("\n");

	if (nBytes >= 5 && strncmp(buffer, "<ACK>", nBytes) == 0) { 
		return EXIT_SUCCESS; 
	}
	else {
		serial.closeDevice();
		return EXIT_FAILURE;
	}
}


int slmx4::openRadar()
{
	serial.writeString("OpenRadar(X4)");

	if(checkACK() == EXIT_SUCCESS) {
		#ifdef DEBUG
 		fprintf(stderr, "SUCCESS: ACK from 'openRadar'\n");
//		sendosc(string_, (void*)"SUCCESS: ACK from 'OpenRadar'", host_ip);
		#endif
	}
 	else { 
		#ifdef DEBUG
		fprintf(stderr, "ERROR: Reading ACK in 'openRadar'\n");
//		sendosc(string_, (void*)"ERROR: Reading ACK in 'OpenRadar'\n", host_ip);
		#endif
		return EXIT_FAILURE;
	}

	updateNumberOfSamplers();

	return EXIT_SUCCESS;
}

void slmx4::closeRadar()
{
	serial.flushReceiver();
	serial.writeString("Close()");

	if(checkACK() == EXIT_SUCCESS) {
		#ifdef DEBUG
 		fprintf(stderr, "SUCCESS: ACK from 'closeRadar'\n");
//		sendosc(string_, (void*)"SUCCESS: ACK from 'CloseRadar'", host_ip);
		#endif
	}
 	else { 
		#ifdef DEBUG
		fprintf(stderr, "ERROR: Reading ACK in 'closeRadar'\n");
//		sendosc(string_, (void*)"ERROR: Reading ACK in 'CloseRadar'\n", host_ip);
		#endif
	}

}


void slmx4::updateNumberOfSamplers()
{
	char buffer[MAX_BYTES_READ];

	serial.writeString("VarGetValue_ByName(SamplersPerFrame)");

	usleep(10);

	serial.readString(buffer, '0', MAX_BYTES_READ-1, TIMEOUT_MS);

	char* token = strtok(buffer, "<");
	if (token != NULL) {
		numSamplers = atoi(token);
	}
	else {
		fprintf(stderr, "ERROR reading number of samplers\n");
	}

	#ifdef DEBUG
	printf("SamplersPerFrame = %d\n",numSamplers);
	#endif
}

int slmx4::iterations()
{
	char buffer[MAX_BYTES_READ];

	serial.writeString("VarGetValue_ByName(Iterations)");

	serial.readString(buffer, '0', MAX_BYTES_READ-1, TIMEOUT_MS);

	char* token = strtok(buffer, "<");
	int iterations = 0;
	if (token != NULL) {
		iterations = atoi(token);
	}
	else {
		fprintf(stderr, "ERROR reading number of Iterations\n");
	}

#ifdef DEBUG
	printf("Iterations: %i\n", iterations);
#endif
	return iterations;
}

int slmx4::setRegister(int cmd)
{

	switch(cmd) {
	case rx_wait:
		serial.writeString("VarSetValue_ByName(rx_wait,0)");
		break;
	case frame_start:
		serial.writeString("VarSetValue_ByName(frame_start,0.3)");
		break;
	case frame_end:
		serial.writeString("VarSetValue_ByName(frame_end,4)");
		break;
	case ddc_en:
		serial.writeString("VarSetValue_ByName(ddc_en,0)");
		break;
	case pps:
		serial.writeString("VarSetValue_ByName(PPS,30)");
		break;
	}

	if(checkACK() == EXIT_SUCCESS) {
		#ifdef DEBUG
 		fprintf(stderr, "SUCCESS: ACK from 'setRegister'\n");
//		sendosc(string_, (void*)"SUCCESS: ACK from 'TryUpdateChip'", host_ip);
		#endif
	}
 	else { 
		#ifdef DEBUG
		fprintf(stderr, "ERROR: Reading ACK in 'setRegister'\n");
//		sendosc(string_, (void*)"ERROR: Reading ACK in 'TryUpdateChip'\n", host_ip);
		#endif
		return EXIT_FAILURE;
	}

    updateNumberOfSamplers();

	return EXIT_SUCCESS;
}

int slmx4::getFrameRaw(_Float32* frame)
{
	serial.flushReceiver();
	int frameSize = numSamplers;
	int nBytesInReceiveBuffer = 0;
	
	serial.writeString("GetFrameRaw()");
	
	timeOut timer;
	timer.initTimer();

	while (1) {
		nBytesInReceiveBuffer = serial.available();
		if (timer.elapsedTime_ms() > TIMEOUT_MS) {
			fprintf(stderr, "Timeout in getFrameRaw\n");
			break;
		}
		if (nBytesInReceiveBuffer >= frameSize*4) {
			serial.readBytes(frame, frameSize * 4, 1);
			break;
		}
	}

	/* #ifdef DEBUG
	for (int j = 0; j < frameSize -1; ++j)
		printf("%f, ", frame[j]);
	#endif */


	if(checkACK() == EXIT_SUCCESS) {
		#ifdef DEBUG
 		fprintf(stderr, "SUCCESS: ACK from 'getFrameRaw'\n");
//		sendosc(string_, (void*)"SUCCESS: ACK from 'getFrameRaw'", host_ip);
		#endif
	}
 	else { 
		#ifdef DEBUG
		fprintf(stderr, "ERROR: Reading ACK in 'getFrameRaw'\n");
//		sendosc(string_, (void*)"ERROR: Reading ACK in 'getFrameRaw'\n", host_ip);
		#endif
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int slmx4::getFrameNormalized(_Float32* frame)
{
	serial.flushReceiver();
	int frameSize = numSamplers;
	int nBytesInReceiveBuffer = 0;
	
	serial.writeString("GetFrameNormalized()");
	
	timeOut timer;
	timer.initTimer();

	while(1) {
		nBytesInReceiveBuffer = serial.available();
		if (timer.elapsedTime_ms() > TIMEOUT_MS) {
			printf("Timeout\n");
			break;
		}
		if(nBytesInReceiveBuffer >= frameSize*4) {
			serial.readBytes(frame, frameSize * 4, 1); 
			break;
		}
	}

/*	#ifdef DEBUG
	for(int j = 0; j < frameSize -1; ++j)
		printf("%f, ", frame[j]);
	#endif */

	if(checkACK() == EXIT_SUCCESS) {
		#ifdef DEBUG
 		fprintf(stderr, "SUCCESS: ACK from 'getFrameNormalized'\n");
//		sendosc(string_, (void*)"SUCCESS: ACK from 'getFrameNormalized'", host_ip);
		#endif
	}
 	else { 
		#ifdef DEBUG
		fprintf(stderr, "ERROR: Reading ACK in 'getFrameNormalized'\n");
//		sendosc(string_, (void*)"ERROR: Reading ACK in 'getframenormalized'\n", host_ip);
		#endif
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*void slmx4::setHost(const char *addr)
{
	strcpy(host_ip, addr);
} */

void slmx4::end()
{
	closeRadar();
	closeSerial();
}

void slmx4::flushSerial() 
{
	serial.flushReceiver();
}

void slmx4::closeSerial() 
{
	serial.closeDevice();
}

