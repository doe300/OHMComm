/* 
 * File:   ReceiveThread.h
 * Author: daniel
 *
 * Created on May 16, 2015, 12:49 PM
 */

#ifndef RTPLISTENER_H
#define	RTPLISTENER_H

#include <thread>

#include "RTPBuffer.h"

#include "NetworkWrapper.h"
#include "configuration.h"

class RTPListener
{
public:
    RTPListener(NetworkWrapper *wrapper, std::unique_ptr<RTPBuffer> *buffer, unsigned int receiveBufferSize);
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
    NetworkWrapper *wrapper;
    std::unique_ptr<RTPBuffer> *buffer;
    RTPPackage *receivedPackage;
    std::thread receiveThread;
    bool threadRunning = false;
    
    /*!
     * Method called in the parallel thread, receiving packages and writing them into RTPBuffer
     */
    void runThread();
};

#endif	/* RTPLISTENER_H */

