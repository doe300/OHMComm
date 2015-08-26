#ifndef UDPWRAPPER_H
#define	UDPWRAPPER_H

#include "configuration.h"
#include "NetworkWrapper.h"
#include <string.h> //for strerror

/*!
 * NetworkWrapper implementation using the UDP protocol
 */
class UDPWrapper : public NetworkWrapper
{
public:
    UDPWrapper(unsigned short portIncoming, const std::string remoteIPAddress, unsigned short portOutgoing);

    UDPWrapper(const NetworkConfiguration& networkConfig);

    ~UDPWrapper();

    int sendData(void *buffer, unsigned int bufferSize = 0);
    int receiveData(void *buffer, unsigned int bufferSize = 0);

    void closeNetwork();
    std::wstring getLastError() const;
private:
    bool isIPv6;
    int Socket;
    sockaddr localAddress;
    sockaddr remoteAddress;

    void startWinsock();

    void createSocket();

    void initializeNetwork();

    void initializeNetworkConfig(unsigned short localPort, const std::string remoteAddress, unsigned short remotePort);
};



#endif
