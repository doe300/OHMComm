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

    int sendData(const void *buffer, const unsigned int bufferSize = 0);
    int receiveData(void *buffer, unsigned int bufferSize = 0);

    void closeNetwork();
    std::wstring getLastError() const;
private:
    bool isIPv6;
    int Socket;
    //we define a union of an IPv4 and an IPv6 address
    //because the two addresses have different size(16 bytes and 24 bytes) and therefore we can guarantee to hold enough space
    //for either of the IP protocol versions
    union socketAddress
    {
        sockaddr_in6 ipv6;
        sockaddr_in ipv4;
    };
    socketAddress localAddress;
    socketAddress remoteAddress;

    void startWinsock();

    bool createSocket();

    void initializeNetwork();

    void initializeNetworkConfig(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort);
    
    /*!
     * \returns the size of the socket-address depending on the IP-version used
     */
    const int getSocketAddressLength();
};



#endif
