/*
 * File:   ReceiveThread.cpp
 * Author: daniel
 *
 * Created on May 16, 2015, 12:49 PM
 */

#include "rtp/RTPListener.h"
#include "Statistics.h"

RTPListener::RTPListener(std::shared_ptr<NetworkWrapper> wrapper, std::shared_ptr<RTPBufferHandler> buffer, unsigned int receiveBufferSize) :
    rtpHandler(receiveBufferSize)
{
    this->wrapper = wrapper;
    this->buffer = buffer;
}

RTPListener::RTPListener(const RTPListener& orig) : rtpHandler(orig.rtpHandler)
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
    if(!threadRunning)
    {
        threadRunning = true;
        receiveThread = std::thread(&RTPListener::runThread, this);
    }
}

void RTPListener::runThread()
{
    std::cout << "RTP-Listener started ..." << std::endl;
    while(threadRunning)
    {
        //1. wait for package and store into RTPPackage
        const NetworkWrapper::Package receivedPackage = this->wrapper->receiveData(rtpHandler.getWorkBuffer(), rtpHandler.getMaximumPackageSize());
        if(receivedPackage.isInvalidSocket())
        {
            //socket was already closed
            shutdown();
        }
        else if(receivedPackage.hasTimedOut())
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        else if(threadRunning && RTPPackageHandler::isRTPPackage(rtpHandler.getWorkBuffer(), receivedPackage.getReceivedSize()))
        {
            //2. write package to buffer
            auto result = buffer->addPackage(rtpHandler, receivedPackage.getReceivedSize() - RTPHeader::MIN_HEADER_SIZE);
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
                Participant& participant = ParticipantDatabase::remote(rtpHandler.getRTPPackageHeader()->getSSRC());
                //on first package from remote, set values
                if(participant.payloadType == PayloadType::ALL)
                {
                    participant.payloadType = rtpHandler.getRTPPackageHeader()->getPayloadType();
                    //set initial extended highest sequence number
                    participant.extendedHighestSequenceNumber = rtpHandler.getRTPPackageHeader()->getSequenceNumber();
                    participant.initialRTPTimestamp = rtpHandler.getRTPPackageHeader()->getTimestamp();
                    std::cout << "RTP: New remote joined conversation: " << rtpHandler.getRTPPackageHeader()->getSSRC() << std::endl;
                }
                else if(participant.payloadType != rtpHandler.getRTPPackageHeader()->getPayloadType())
                {
                    std::cerr << "RTP: Invalid payload-type for remote! " << std::endl;
                }
                else    //set extended highest sequence number
                {
                    participant.extendedHighestSequenceNumber  = calculateExtendedHighestSequenceNumber(participant, rtpHandler.getRTPPackageHeader()->getSequenceNumber());
                }
                participant.lastPackageReceived = std::chrono::steady_clock::now();
                participant.totalPackages += 1;
                participant.totalBytes += receivedPackage.getReceivedSize();
                participant.calculateInterarrivalJitter(rtpHandler.getRTPPackageHeader()->getTimestamp(), rtpHandler.getCurrentRTPTimestamp());
                
                Statistics::incrementCounter(Statistics::COUNTER_PACKAGES_RECEIVED, 1);
                Statistics::incrementCounter(Statistics::COUNTER_HEADER_BYTES_RECEIVED, RTPHeader::MIN_HEADER_SIZE);
                Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_RECEIVED, receivedPackage.getReceivedSize() - RTPHeader::MIN_HEADER_SIZE);
            }
        }
    }
    std::cout << "RTP-Listener shut down" << std::endl;
}

void RTPListener::shutdown()
{
    // notify the thread to stop
    threadRunning = false;
}

uint32_t RTPListener::calculateExtendedHighestSequenceNumber(const Participant& participant, const uint16_t receivedSequenceNumber)
{
    //See https://tools.ietf.org/html/rfc3711#section-3.3.1
    const uint32_t previousValue = participant.extendedHighestSequenceNumber;
    //rollover-count is the higher 16 bits
    const uint32_t rollOverCount = previousValue >> 16;
    //determine possible values for the next extended highest sequence number
    const uint32_t possibleValues[3] = {
        //package is older than current EHSeqNr
        ((rollOverCount - 1) << 16) + receivedSequenceNumber,
        //package is in same roll-over than current EHSeqNr
        (rollOverCount << 16) + receivedSequenceNumber,
        //roll-over between current EHSeqNr and new package
        ((rollOverCount + 1) << 16) + receivedSequenceNumber
    };
    //determine closest of the possible values
    //skip ROC-1 unless we actually have a roll over
    uint8_t index = previousValue < UINT16_MAX ? 1 : 0;
    for(uint8_t i = 1; i < 3; i++)
    {
        uint32_t diffPrev = std::abs((int32_t)(possibleValues[index] - previousValue));
        uint32_t diffNew = std::abs((int32_t)(possibleValues[i] - previousValue));
        if(diffNew < diffPrev)
        {
            index = i;
        }
    }
    return possibleValues[index];
}
