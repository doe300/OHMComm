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

// Defines OS-independant flag to close socket
#define SHUTDOWN_BOTH SD_BOTH   // 2
#else
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h> // sockaddr_in
#include <stdexcept>
#include <unistd.h> //socklen_t

// Defines OS-independant flag to close socket
#define SHUTDOWN_BOTH SHUT_RDWR // 2
#endif

/*!
 * NetworkWrapper implementation using the UDP protocol
 */
class UDPWrapper : public NetworkWrapper
{
public:
    UDPWrapper(sockaddr_in localAddress, sockaddr_in remoteAddress);

    UDPWrapper(std::string localIPAddress, unsigned short portIncoming, std::string remoteIPAddress, unsigned short portOutgoing);

    UDPWrapper(struct NetworkConfiguration networkConfig);

    void initializeNetwork();

    void startWinsock();

    void createSocket();

    void InitializeNetworkConfig(std::string addressIncoming, unsigned short portIncoming, std::string addressOutgoing, unsigned short portOutgoing);

    int sendDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0);
    int recvDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0);

    void closeSocket(int fd);
    int getLastError();
protected:
    int Socket;
    sockaddr_in localAddress;
    sockaddr_in remoteAddress;
    unsigned int outputBufferSize = 0;
    unsigned int inputBufferSize = 0;
};



#endif