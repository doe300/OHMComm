#ifndef NETWORKWRAPPER_H
#define	NETWORKWRAPPER_H

#include <iostream>
#include <string>
#include <string.h> //for strerror

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
        };

        /*!
         * Superclass for all networking-protocols.
         *
         * Implementations of this class provide methods to send/receive audio-data to/from the network.
         *
         */
        class NetworkWrapper
        {
        public:

            const static int RECEIVE_TIMEOUT{-2};

            //Data-object containing information about a single package received

            struct Package
            {
                SocketAddress address;
                //error-code or package-size
                int status;

                /*!
                 * \return whether the call to receiveData() has timed out
                 */
                inline bool hasTimedOut() const
                {
                    return status == RECEIVE_TIMEOUT;
                }

                inline bool isInvalidSocket() const
                {
                    return status == INVALID_SOCKET;
                }

                inline unsigned int getReceivedSize() const
                {
                    return status > 0 ? status : 0;
                }
            };

            virtual ~NetworkWrapper()
            {
                //needs a virtual destructor to be overridden correctly
            }

            /*!
             * \param buffer The buffer to send
             *
             * \param bufferSize The number of bytes to send
             *
             * \return the number of bytes sent
             */
            virtual int sendData(const void *buffer, const unsigned int bufferSize) = 0;

            /*!
             * In case of an error, this method returns INVALID_SOCKET. In case of a blocking-timeout, this method returns RECEIVE_TIMEOUT
             * 
             * \param buffer The buffer to receive into
             *
             * \param bufferSize The maximum number of bytes to receive
             *
             * \return information about the received package
             */
            virtual Package receiveData(void *buffer, unsigned int bufferSize) = 0;

            /*!
             * Returns the last error code and a human-readable description
             */
            virtual std::wstring getLastError() const;

            /*!
             * Closes the underlying socket
             */
            virtual void closeNetwork() = 0;

            /*!
             * \param ipAddress The address to check
             *
             * \return Whether the address given is an IPv6 address
             */
            static bool isIPv6(const std::string ipAddress);
        protected:

            // Defines OS-independant flag to close socket
#ifdef _WIN32
            static constexpr int SHUTDOWN_BOTH{SD_BOTH};
            static constexpr int INTERRUPTED_BY_SYSTEM_CALL{WSAEINTR};
#else
            static constexpr int SHUTDOWN_BOTH{SHUT_RDWR};
            static constexpr int INTERRUPTED_BY_SYSTEM_CALL{EINTR};
#endif

            /*!
             * \return whether the recv()-method has returned because of a timeout
             */
            bool hasTimedOut() const;
        };
    }
}
#endif
