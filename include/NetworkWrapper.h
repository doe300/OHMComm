#ifndef NETWORKWRAPPER_H
#define	NETWORKWRAPPER_H

#include <iostream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

// Defines OS-independant flag to close socket
#define SHUTDOWN_BOTH SD_BOTH   // 2
#else
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h> // sockaddr_in
#include <stdexcept>
#include <unistd.h> //socklen_t

// Defines OS-independant flag to close socket
#define SHUTDOWN_BOTH SHUT_RDWR // 2
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define WSAETIMEDOUT 10060  //Dummy-support for WSAPI timeout-error
#endif

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

    virtual ~NetworkWrapper()
    {
        //needs a virtual destructor to be overridden correctly
    }

    /*!
     * \param buffer The buffer to send
     *
     * \param bufferSize The number of bytes to send
     *
     * Returns the number of bytes sent
     */
    virtual int sendData(const void *buffer, const unsigned int bufferSize = 0) = 0;

    /*!
     * In case of an error, this method returns INVALID_SOCKET. In case of a blocking-timeout, this method returns RECEIVE_TIMEOUT
     * 
     * \param buffer The buffer to receive into
     *
     * \param bufferSize The maximum number of bytes to receive
     *
     * Returns the number of bytes received
     */
    virtual int receiveData(void *buffer, unsigned int bufferSize = 0) = 0;

    /*!
     * Returns the last error code and a human-readable description
     */
    virtual std::wstring getLastError() const = 0;

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
    
    /*!
     * \return whether the recv()-method has returned because of a timeout
     */
    bool hasTimedOut() const;
};

#endif
