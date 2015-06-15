#ifndef PROCESSORRTP_H

#define	PROCESSORRTP_H
#include <string>
#include "AudioProcessor.h"
#include "RTPPackage.h"
#include "NetworkWrapper.h"
#include "RTPBuffer.h"

/*!
 * AudioProcessor wrapping/unwrapping audio-frames in/out of a RTP-package
 * 
 * This implementation uses a seperate thread (see RTPListener) to receive packages.
 * This was implemented to avoid blocking the audio-loop while waiting for packages.
 * The received RTPPackages are buffered in an RTPBuffer
 */
class ProcessorRTP : public AudioProcessor
{
public:
    /*!
     * Constructs a new ProcessorRTP
     * 
     * \param name The name of the AudioProcessor
     * 
     * \param networkwrapper The NetworkWrapper to use sending packages
     * 
     * \param buffer The RTPBuffer to read packages from
     */
    ProcessorRTP(std::string name, NetworkWrapper *networkwrapper, std::unique_ptr<RTPBuffer> *buffer);
    
    void processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
    void processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);
private:
    NetworkWrapper *networkObject;
    RTPPackage *rtpPackage = NULL;
    std::unique_ptr<RTPBuffer> *rtpBuffer;
};
#endif