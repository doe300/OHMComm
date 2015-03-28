/* 
 * File:   RTPWrapper.cpp
 * Author: daniel
 * 
 * Created on March 7, 2015, 1:10 PM
 */

#include <stdlib.h>

#include "RTPWrapper.h"

RTPWrapper::RTPWrapper(int socket, uint8_t payloadType, int16_t tickInterval)
{
    //initialize random start values
    synchronizationSource = generateSynchronizationSource();
    currentSequenceNumber = generateSequenceNumber();
    //set all values to NULL
    contributionSources.fill(0);
}

RTPWrapper::RTPWrapper(const RTPWrapper& orig)
{
    synchronizationSource = orig.synchronizationSource;
    currentSequenceNumber = orig.currentSequenceNumber;
    //copies the other wrapper's contribution sources
    contributionSources = orig.contributionSources;
}

////
// Send/Receive
////

int inout(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, unsigned int status, void* data)
{
    //TODO:
    /*
     * 1. send the inputBuffer over RTP
     * 1.1 split into packages (one per frame)
     * 1.2 build header
     * 1.3 write into stream
     * 2. read from underlying stream/buffer and store data in outputBuffer
     * 2.1 read from stream/buffer
     * 2.2 extract headers
     * 2.3 split into packages (one per frame)
     * 2.4 reorder packages
     * 2.5 fill buffer
     * 
     * Mind the data-type of the stream, defined at stream startup (see RTAudio.h)
     * 
     * 3. return-value to close/continue playback
     */
    return 2;
}

////
// Sender configuration
////

int8_t RTPWrapper::addContributionSource(uint32_t csrc)
{
    if(getContributingSourcesCount() < 15)
    {
        for(uint8_t i = 0;i < contributionSources.size(); i++)
        {
            if(contributionSources[i] == 0)
            {
                contributionSources[i] = csrc;
                return 0;
            }
        }
    }
    return -1;
}

uint8_t RTPWrapper::getContributingSourcesCount()
{
    uint8_t number = 0;
    for(uint8_t i = 0; i < contributionSources.size();i++)
    {
        if(contributionSources[i] != 0)
        {
            number ++;
        }
    }
    return number;
}

uint16_t RTPWrapper::getSequenceNumber()
{
    return currentSequenceNumber;
}

uint32_t RTPWrapper::getSynchronizationSource()
{
    return synchronizationSource;
}


////
// private helper methods
////

uint16_t RTPWrapper::generateSequenceNumber()
{
    //TODO better random??
    return rand() & 0xFFFF;
}

uint32_t RTPWrapper::generateSynchronizationSource()
{
    //TODO better random??
    return rand() & 0xFFFFFFFF;
}

uint32_t RTPWrapper::generateTimestamp()
{
    //TODO better random??
    return rand() & 0xFFFFFFFF;
}

uint16_t RTPWrapper::incrementSequenceNumber()
{
    //TODO overflow handling - required?
    return currentSequenceNumber++;
}
