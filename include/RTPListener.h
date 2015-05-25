/* 
 * File:   ReceiveThread.h
 * Author: daniel
 *
 * Created on May 16, 2015, 12:49 PM
 */

#ifndef RTPLISTENER_H
#define	RTPLISTENER_H

#include <thread>
#include <sys/socket.h>
#include <malloc.h>

#include "RTPBuffer.h"

#include "NetworkWrapper.h"
#include "configuration.h"

class RTPListener
{
public:
    RTPListener(const sockaddr *receiveAddress, const RTPBuffer *buffer);
    RTPListener(const RTPListener& orig);
    virtual ~RTPListener();
    
    /*!
     * Shuts down the receive-thread
     */
    void shutdown();
    
    /*!
     * Starts the receive-thread
     */
    void startUp();
private:
    /*!
     * The socket to listen on
     */
    int receiveSocket;
    const RTPBuffer *buffer;
    const sockaddr *receiveAddress;
    std::thread receiveThread;
    bool threadRunning = false;
    char *receiveBuffer;
    
    /*!
     * Method called in the parallel thread, receiving packages and writing them into RTPBuffer
     */
    void runThread();
};

#endif	/* RTPLISTENER_H */

