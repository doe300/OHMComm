#ifndef PROCESSORRTP_H
#define	PROCESSORRTP_H

#include "ParticipantDatabase.h"
#include "AudioProcessor.h"
#include "RTPPackageHandler.h"
#include "network/NetworkWrapper.h"
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
    bool configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode);

    unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
    unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

    bool cleanUp();
private:
    //! Delay until treating input as silence
    //!Treat as silence after 500ms of no input
    static constexpr unsigned short SILENCE_DELAY{500};
    const std::shared_ptr<NetworkWrapper> network;
    const std::shared_ptr<RTPBufferHandler> rtpBuffer;
    Participant& ourselves;
    std::unique_ptr<RTPPackageHandler> rtpPackage;
    bool isDTXEnabled;
    bool lastPackageWasSilent;
    unsigned short totalSilenceDelayPackages;
    unsigned int currentSilenceDelayPackages;
    
    void initPackageHandler(unsigned int maxBufferSize);
};
#endif
