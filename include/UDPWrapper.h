#ifndef UDPWRAPPER_H
#define	UDPWRAPPER_H

//#include <sstream>
//#include <iostream>
//#include <stdio.h>
#include "configuration.h"
#include "NetworkWrapper.h"
#include <string.h> //for memcpy


#ifdef _WIN32
#include <winsock2.h>
//#include <cstdint>
#else
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h> // sockaddr_in
#include <stdexcept>
#include <unistd.h> //socklen_t
#endif

class UDPWrapper : public NetworkWrapper
{
public:
	UDPWrapper(sockaddr_in addressDataIncoming, sockaddr_in addressDataOutgoing, unsigned int outputBufferSize, unsigned int inputBufferSize);

	UDPWrapper(std::string addressIncoming, unsigned short portIncoming, std::string addressOutgoing, unsigned short portOutgoing, unsigned int outputBufferSize, unsigned int inputBufferSize);

	UDPWrapper(struct NetworkConfiguration networkConfig);

	void initializeNetwork();

	void startWinsock();

	void createSocket();

	void InitializeNetworkConfig(std::string addressIncoming, unsigned short portIncoming, std::string addressOutgoing, unsigned short portOutgoing);

	int sendDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0);
	int recvDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0);
protected:
	int Socket;
    sockaddr_in addressDataIncoming;
	sockaddr_in addressDataOutgoing;
	unsigned int outputBufferSize = 0;
	unsigned int inputBufferSize = 0;
};



#endif