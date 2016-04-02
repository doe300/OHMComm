/* 
 * File:   SocketAddress.h
 * Author: daniel
 *
 * Created on April 2, 2016, 10:44 AM
 */

#ifndef SOCKETADDRESS_H
#define	SOCKETADDRESS_H

#include <utility>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h> // sockaddr_in
#include <stdexcept>
#include <unistd.h> //socklen_t
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define WSAETIMEDOUT 10060  //Dummy-support for WSAPI timeout-error
#endif

namespace ohmcomm
{
    namespace network
    {

        /*!
         * Container for a socket-address (IP-address and port) and a flag whether to use IPv4 or IPv6
         */
        struct SocketAddress
        {
        public:
            //we define a union of an IPv4 and an IPv6 address
            //because the two addresses have different size(16 bytes and 24 bytes) and therefore we can guarantee to hold enough space
            //for either of the IP protocol versions

            union
            {
                sockaddr_in6 ipv6;
                sockaddr_in ipv4;
            };
            bool isIPv6;
            
            /*!
             * Returns the IP-address and the port as several variables
             */
            std::pair<std::string, uint16_t> toAddressAndPort() const;
            
            /*!
             * Creates a new SocketAddress from the given host-address and port
             * 
             * \param address The host-address (IP-address or domain-name)
             * 
             * \param port The port-number
             */
            static SocketAddress fromAddressAndPort(const std::string& address, const uint16_t port);
            
            /*!
             * Creates a new local SocketAddress, listening on any local IP-address and the given port
             * 
             * \param isIPv6 Whether to create an IPv6 address
             * 
             * \param localPort The local port to listen on
             */
            static SocketAddress createLocalAddress(const bool isIPv6, const uint16_t localPort);
        };
    }
}
#endif	/* SOCKETADDRESS_H */

