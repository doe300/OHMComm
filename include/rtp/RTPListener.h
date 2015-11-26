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

#include "ParticipantDatabase.h"
#include "RTPBufferHandler.h"
#include "NetworkWrapper.h"

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
    RTPListener(std::shared_ptr<NetworkWrapper> wrapper, std::shared_ptr<RTPBufferHandler> buffer, unsigned int receiveBufferSize, std::function<void ()> stopCallback);
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
    std::shared_ptr<RTPBufferHandler> buffer;
    RTPPackageHandler rtpHandler;
    std::thread receiveThread;
    bool threadRunning = false;
    //for jitter-calculation
    uint32_t lastDelay;

    /*!
     * Method called in the parallel thread, receiving packages and writing them into RTPBuffer
     */
    void runThread();
    
    /*!
     * NOTE: is only called from #runThread()
     * 
     * \param sentTimestamp the RTP-timestamp of the remote device read from the RTPHeader
     * 
     * \param receptionTimestamp the RTP-timestamp of this device of the moment of reception
     * 
     * \return the interarrival-jitter for RTP-packages
     */
    float calculateInterarrivalJitter(uint32_t sentTimestamp, uint32_t receptionTimestamp);
};

#endif	/* RTPLISTENER_H */

