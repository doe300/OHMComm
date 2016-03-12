#ifndef UDPWRAPPER_H
#define	UDPWRAPPER_H

#include "configuration.h"
#include "NetworkWrapper.h"

namespace ohmcomm
{

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
        Package receiveData(void *buffer, unsigned int bufferSize = 0);

        void closeNetwork();
    protected:
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
        int getSocketAddressLength();
    };
}


#endif
