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
    //we define them as IPv6-addresses, because it is the greater container-format (24 instead of 16 bytes) and is guaranteed to hold IPv4 addresses
    sockaddr_in6 localAddress;
    sockaddr_in6 remoteAddress;

    void startWinsock();

    void createSocket();

    void initializeNetwork();

    void initializeNetworkConfig(unsigned short localPort, const std::string remoteAddress, unsigned short remotePort);
    
    /*!
     * \returns the size of the socket-address depending on the IP-version used
     */
    const int getSocketAddressLength();
};



#endif
