/*
 * File:   ReceiveThread.cpp
 * Author: daniel
 *
 * Created on May 16, 2015, 12:49 PM
 */

#include "RTPListener.h"
#include "Statistics.h"

RTPListener::RTPListener(std::shared_ptr<NetworkWrapper> wrapper, std::shared_ptr<RTPBufferHandler> buffer, unsigned int receiveBufferSize, std::function<void()> stopCallback) :
    stopCallback(stopCallback), rtpHandler(receiveBufferSize), lastDelay(0)
{
    this->wrapper = wrapper;
    this->buffer = buffer;
}

RTPListener::RTPListener(const RTPListener& orig) : rtpHandler(orig.rtpHandler), lastDelay(orig.lastDelay)
{
    this->wrapper = orig.wrapper;
    this->buffer = orig.buffer;
}

RTPListener::~RTPListener()
{
    // Wait until thread has really stopped
    receiveThread.join();
}

void RTPListener::startUp()
{
    threadRunning = true;
    receiveThread = std::thread(&RTPListener::runThread, this);
}

void RTPListener::runThread()
{
    std::cout << "RTP-Listener started ..." << std::endl;
    participantDatabase[PARTICIPANT_REMOTE] = {0};
    while(threadRunning)
    {
        //1. wait for package and store into RTPPackage
        int receivedSize = this->wrapper->receiveData(rtpHandler.getWorkBuffer(), rtpHandler.getMaximumPackageSize());
        if(receivedSize == INVALID_SOCKET)
        {
            //socket was already closed
            shutdown();
        }
        else if(receivedSize == NetworkWrapper::RECEIVE_TIMEOUT)
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        else if(threadRunning && RTPPackageHandler::isRTPPackage(rtpHandler.getWorkBuffer(), (unsigned int)receivedSize))
        {
            //2. write package to buffer
            auto result = buffer->addPackage(rtpHandler, receivedSize - RTP_HEADER_MIN_SIZE);
            if (result == RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW)
            {
                //TODO some handling or simply discard?
                std::cerr << "Input Buffer overflow" << std::endl;
            }
            else if (result == RTPBufferStatus::RTP_BUFFER_PACKAGE_TO_OLD)
            {
                std::cerr << "Package was too old, discarding" << std::endl;
            }
            else
            {
                calculateInterarrivalJitter(rtpHandler.getRTPPackageHeader()->getTimestamp(), rtpHandler.getCurrentRTPTimestamp());
                participantDatabase[PARTICIPANT_REMOTE].ssrc = rtpHandler.getRTPPackageHeader()->getSSRC();
                Statistics::incrementCounter(Statistics::COUNTER_PACKAGES_RECEIVED, 1);
                Statistics::incrementCounter(Statistics::COUNTER_HEADER_BYTES_RECEIVED, RTP_HEADER_MIN_SIZE);
                Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_RECEIVED, receivedSize - RTP_HEADER_MIN_SIZE);
            }
        }
    }
    std::cout << "RTP-Listener shut down" << std::endl;
}

float RTPListener::calculateInterarrivalJitter(uint32_t sentTimestamp, uint32_t receptionTimestamp)
{
    //as of RFC 3550 (A.8):
    //D(i, j)=(Rj - Sj) - (Ri - Si)
    //with (Ri - Si) = lastDelay
    int32_t currentDelay = receptionTimestamp - sentTimestamp;
    int32_t currentDifference = currentDelay - lastDelay;
    lastDelay = currentDelay;
    
    //Ji = Ji-1 + (|D(i-1, 1)| - Ji-1)/16
    double lastJitter = participantDatabase[PARTICIPANT_REMOTE].interarrivalJitter;
    lastJitter = lastJitter + ((float)abs(currentDifference) - lastJitter)/16.0;
    participantDatabase[PARTICIPANT_REMOTE].interarrivalJitter = lastJitter;
    return lastJitter;
}

void RTPListener::shutdown()
{
    // notify the thread to stop
    threadRunning = false;
}

