#ifndef NETWORKWRAPPER_H
#define	NETWORKWRAPPER_H

#include <iostream>
#include <string>

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
    virtual int sendData(void *buffer, unsigned int bufferSize = 0) = 0;

    /*!
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
    virtual std::string getLastError() = 0;

    /*!
     * Closes the underlying socket
     */
    virtual void closeNetwork() = 0;
    
protected:
    /*!
     * \param ipAddress The address to check
     * 
     * \return Whether the address given is an IPv6 address
     */
    static bool isIPv6(std::string ipAddress);
};

#endif