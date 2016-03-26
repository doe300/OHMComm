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

            int sendData(const void *buffer, const unsigned int bufferSize = 0) override;
            Package receiveData(void *buffer, unsigned int bufferSize = 0) override;

            void closeNetwork();
        private:

            int Socket;
            SocketAddress localAddress;
            SocketAddress remoteAddress;

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

