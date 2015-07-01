#ifndef PROCESSORRTP_H

#define	PROCESSORRTP_H
#include "AudioProcessor.h"
#include "RTPPackageHandler.h"
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
    
    unsigned int getSupportedAudioFormats();
    unsigned int getSupportedSampleRates();
    std::vector<int> getSupportedBufferSizes(unsigned int sampleRate);

    unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
    unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);
private:
    NetworkWrapper *networkObject;
    RTPPackageHandler *rtpPackage = nullptr;
    std::unique_ptr<RTPBuffer> *rtpBuffer;
};
#endif