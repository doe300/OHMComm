#ifndef NETWORKWRAPPER_H
#define	NETWORKWRAPPER_H

#include <iostream>

//defines for non-Windows
#ifndef _WIN32
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
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
    /*!
     * \param buffer The buffer to send
     * 
     * \param bufferSize The number of bytes to send
     * 
     * Returns the number of bytes sent
     */
    virtual int sendDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0) = 0;

    /*!
     * \param buffer The buffer to receive into
     * 
     * \param bufferSize The maximum number of bytes to receive
     * 
     * Returns the number of bytes received
     */
    virtual int recvDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0) = 0;

    /*!
     * Returns the last error code
     */
    virtual int getLastError() = 0;

    /*!
     * Closes the underlying socket
     */
    virtual void closeSocket(int fd) = 0;
};

#endif