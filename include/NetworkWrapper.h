/* 
 * File:   NetworkWrapper.h
 * Author: daniel
 *
 * Created on April 1, 2015, 6:19 PM
 */

#ifndef NETWORKWRAPPER_H
#define	NETWORKWRAPPER_H

#include <sstream>
#include <iostream>
#include <stdio.h>
#ifdef __linux__
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h> // sockaddr_in
#include <stdexcept>
#else
#include <winsock2.h>
#endif

#include "AudioProcessor.h"

class NetworkWrapper : public AudioProcessor
{
public:
    NetworkWrapper();
    NetworkWrapper(const NetworkWrapper& orig);
    virtual ~NetworkWrapper();
    
    /*!
     * Initializes the network-connection
     * 
     * Returns zero on success and the error-code on error
     */
    virtual int initializeNetwork();
protected:
    int Socket = -1;
    
    /*!
     * Starts Winsock2 for Windows OS
     * 
     * Returns zero on success and the error-code on error
     */
    int startWinsock();
    
    /*!
     * Creates a socket from the global networkConfiguration
     * 
     * Returns -1 on error or the socket-descriptor on success
     */
    int createSocket();
};

#endif	/* NETWORKWRAPPER_H */

