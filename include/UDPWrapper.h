#ifndef UDPWRAPPER_H
#define	UDPWRAPPER_H

//#include <sstream>
//#include <iostream>
//#include <stdio.h>
#include "configuration.h"
#include "NetworkWrapper.h"
#include <string.h> //for memcpy

/*!
 * NetworkWrapper implementation using the UDP protocol
 */
class UDPWrapper : public NetworkWrapper
{
public:
    UDPWrapper(unsigned short portIncoming, std::string remoteIPAddress, unsigned short portOutgoing);

    UDPWrapper(struct NetworkConfiguration networkConfig);

    ~UDPWrapper();
    
    void initializeNetwork();

    void startWinsock();

    void createSocket();

    void initializeNetworkConfig(unsigned short localPort, std::string remoteAddress, unsigned short remotePort);

    int sendData(void *buffer, unsigned int bufferSize = 0);
    int receiveData(void *buffer, unsigned int bufferSize = 0);

    void closeNetwork();
    std::wstring getLastError();
protected:
    bool isIPv6;
    int Socket;
    sockaddr localAddress;
    sockaddr remoteAddress;
    unsigned int outputBufferSize = 0;
    unsigned int inputBufferSize = 0;
};



#endif