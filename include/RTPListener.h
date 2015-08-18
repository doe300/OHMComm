/*
 * File:   ReceiveThread.h
 * Author: daniel
 *
 * Created on May 16, 2015, 12:49 PM
 */

#ifndef RTPLISTENER_H
#define	RTPLISTENER_H

#include <thread>
#include <functional>

#include "RTPBuffer.h"

#include "NetworkWrapper.h"
#include "RTCPPackageHandler.h"

/*!
 * Listening-thread for incoming RTP-packages
 *
 * This class starts a new thread which writes all received RTP-packages to the RTPBuffer
 */
class RTPListener
{
public:
    /*!
     * Constructs a new RTPListener
     *
     * \param wrapper The NetworkWrapper to use for receiving packages
     *
     * \param buffer The RTPBuffer to write into
     *
     * \param receiveBufferSize The maximum size (in bytes) a RTP-package can fill, according to the configuration
     *
     * \param stopCallback The callback to be executed after receiving a RTCP GOODBYE-package
     */
    RTPListener(std::shared_ptr<NetworkWrapper> wrapper, std::shared_ptr<RTPBuffer> buffer, unsigned int receiveBufferSize, std::function<void ()> stopCallback);
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
    std::function<void ()> stopCallback;
    std::shared_ptr<NetworkWrapper> wrapper;
    std::shared_ptr<RTPBuffer> buffer;
    RTPPackageHandler rtpHandler;
    RTCPPackageHandler rtcpHandler;
    std::thread receiveThread;
    bool threadRunning = false;

    /*!
     * Method called in the parallel thread, receiving packages and writing them into RTPBuffer
     */
    void runThread();

    /*!
     * This method handles received RTCP packages and is called only from #runThread()
     */
    void handleRTCPPackage(void* receiveBuffer, unsigned int receivedSize);
};

#endif	/* RTPLISTENER_H */

