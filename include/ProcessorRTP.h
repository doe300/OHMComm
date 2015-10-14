#ifndef PROCESSORRTP_H

#define	PROCESSORRTP_H
#include "AudioProcessor.h"
#include "RTPPackageHandler.h"
#include "NetworkWrapper.h"
#include "RTPBufferHandler.h"

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
     * 
     * \param payloadType The payload-type for the RTP packages
     */
    ProcessorRTP(const std::string name, std::shared_ptr<NetworkWrapper> networkwrapper, 
                 std::shared_ptr<RTPBufferHandler> buffer, const PayloadType payloadType);

    unsigned int getSupportedAudioFormats() const;
    unsigned int getSupportedSampleRates() const;
    const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;

    unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
    unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

    bool cleanUp();
private:
    std::shared_ptr<NetworkWrapper> networkObject;
    RTPPackageHandler *rtpPackage = nullptr;
    std::shared_ptr<RTPBufferHandler> rtpBuffer;
    const PayloadType payloadType;
};
#endif
