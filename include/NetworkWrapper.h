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
#ifdef _WIN32
#include <winsock2.h>
#include <cstdint>
#else
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h> // sockaddr_in
#include <stdexcept>
#endif

#include "AudioProcessor.h"

//Socket-ID for an invalid socket
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
//define SOCKET_ERROR for non-Windows
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

class NetworkWrapper : public AudioProcessor
{
public:
    NetworkWrapper();
    NetworkWrapper(const NetworkWrapper& orig);
    virtual ~NetworkWrapper();
    
protected:
    int Socket;
    
    /*!
     * Initializes the network-connection
     * 
     * Returns zero on success and the error-code on error
     */
    int initializeNetwork();
    
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
    
    /*!
     * \param audioFormat The RtAudioFormat used
     * 
     * Returns the number of bytes per frame of the given audioFormat
     */
    uint8_t getBytesFromAudioFormat(RtAudioFormat audioFormat);
    
    /*!
    * Returns the last error code - depending on the operating system
    */
    int getLastError();
    
    /*!
     * The number of bytes in a input-/output-frame. Use this value to not require to calculate #getBytesFromAudioFormat(format) in every loop-cycle
     */
    uint8_t outputFrameSize, inputFrameSize;
    
    /*!
     * Calculates the size of the used buffer-space
     * 
     * \param numberOfFrames The number of frames, this buffer holds
     * 
     * \param sizeOfFrame The size (in bytes) of one audio-frame
     * 
     * \param numberOfChannels The number of channels in this stream
     */
    unsigned int getBufferSize(unsigned int numberOfFrames, uint8_t sizeOfFrame, uint8_t numberOfChannels);
};

#endif	/* NETWORKWRAPPER_H */

