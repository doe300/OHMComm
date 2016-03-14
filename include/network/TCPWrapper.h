/* 
 * File:   TCPWrapper.h
 * Author: daniel
 *
 * Created on December 10, 2015, 1:22 PM
 */

#ifndef TCPWRAPPER_H
#define	TCPWRAPPER_H
#include "configuration.h"
#include "NetworkWrapper.h"

namespace ohmcomm
{
    namespace network
    {
        /*!
         * NetworkWrapper implementation using the TCP protocol
         */
        class TCPWrapper : public NetworkWrapper
        {
        public:
            TCPWrapper(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort);

            TCPWrapper(const NetworkConfiguration& networkConfig);

            ~TCPWrapper();

            int sendData(const void *buffer, const unsigned int bufferSize = 0);
            Package receiveData(void *buffer, unsigned int bufferSize = 0);

            void closeNetwork();
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
            int getSocketAddressLength();
        };
    }
}
#endif	/* TCPWRAPPER_H */

