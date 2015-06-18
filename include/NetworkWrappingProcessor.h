/* 
 * File:   NetworkWrappingProcessor.h
 * Author: daniel
 *
 * Created on June 14, 2015, 10:05 AM
 */

#ifndef NETWORKWRAPPINGPROCESSOR_H
#define	NETWORKWRAPPINGPROCESSOR_H

#include "AudioProcessor.h"
#include "NetworkWrapper.h"

/*!
 * AudioProcessor which simply forwards the packages to the underlying NetworkWrapper
 */
class NetworkWrappingProcessor : public AudioProcessor
{
public:
    /*!
     * \param wrapper The underlying NetworkWrapper
     */
    NetworkWrappingProcessor(std::string name, NetworkWrapper *wrapper);
    virtual ~NetworkWrappingProcessor();

    virtual void processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);
    virtual void processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);

private:
    NetworkWrapper *wrapper;
};

#endif	/* NETWORKWRAPPINGPROCESSOR_H */

