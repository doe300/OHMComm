#ifndef UDPWRAPPER_H
#define	UDPWRAPPER_H

#include "configuration.h"
#include "NetworkWrapper.h"

namespace ohmcomm
{
    namespace network
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


#endif
