#include <iostream>

#include <OscOutboundPacketStream.h>
#include <UdpSocket.h>

#include "sendosc.h"

#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/*------------------------------------------------------------------------------------*/

#define BUF_SIZE 1024
#define MESSAGE_MAX BUF_SIZE / 2

#define PORT 5555

/*------------------------------------------------------------------------------------*/


//int sendosc(type t, void* val, const char* host)
int sendosc(type t, void* val, const char* host)
{
	using namespace osc;

	//const char host[30] = "bob";

	int i = 0; //var for sine

	// setup udp socket
	UdpTransmitSocket transmitSocket(IpEndpointName(host, PORT));



		// setup packet
		char buf[BUF_SIZE];
		memset(buf, 0, BUF_SIZE);
		osc::OutboundPacketStream p(buf, BUF_SIZE);

		char path[MESSAGE_MAX] = {};


		switch(t)
		{
		case int_:
			strcpy(path, "int");
			p << osc::BeginMessage(path);
			p << *((int*)val);
			p << osc::EndMessage;
			break;
		case string_:
			strcpy(path, (const char*)val);
			p << osc::BeginMessage(path);
			p << osc::EndMessage;
			break;
		case sine_:
			strcpy(path, "int");
			while(1)
			{
				p << osc::BeginMessage(path);
				p << (int)(60*sin(((double)i/150))) + 60;
				p << osc::EndMessage;
				usleep(5000);
				++i;
			}

			break;
		case float_:
			strcpy(path, "float");
			p << osc::BeginMessage(path);
			p << *((float*)val);
			p << osc::EndMessage;
			break;
		}


		transmitSocket.Send( p.Data(), p.Size() );
//	}

	//bundle?

	return 0;
}
