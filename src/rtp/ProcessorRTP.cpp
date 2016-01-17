#include "rtp/ProcessorRTP.h"
#include "Statistics.h"

Participant participantDatabase[2] = {0};

ProcessorRTP::ProcessorRTP(const std::string name, std::shared_ptr<NetworkWrapper> networkwrapper, 
                           std::shared_ptr<RTPBufferHandler> buffer, const PayloadType payloadType) : AudioProcessor(name), payloadType(payloadType)
{
    this->networkObject = networkwrapper;
    this->rtpBuffer = buffer;
}

unsigned int ProcessorRTP::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_ALL;
}

unsigned int ProcessorRTP::getSupportedSampleRates() const
{
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

const std::vector<int> ProcessorRTP::getSupportedBufferSizes(unsigned int sampleRate) const
{
    return std::vector<int>({BUFFER_SIZE_ANY});
}

unsigned int ProcessorRTP::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData)
{
    // pack data into a rtp-package
    if (rtpPackage.get() == nullptr)
    {
        initPackageHandler(userData->maxBufferSize);
    }
    if(userData->isSilentPackage)
    {
        //XXX only check if DTX is enabled
        std::cout << "Not sending silent package" << std::endl;
        return inputBufferByteSize;
    }
    const void* newRTPPackage = rtpPackage->createNewRTPPackage(inputBuffer, inputBufferByteSize);
    //only send the number of bytes really required: header + actual payload-size
    this->networkObject->sendData(newRTPPackage, rtpPackage->getRTPHeaderSize() + inputBufferByteSize);

    participantDatabase[PARTICIPANT_SELF].extendedHighestSequenceNumber += 1;
    Statistics::incrementCounter(Statistics::COUNTER_FRAMES_SENT, userData->nBufferFrames);
    Statistics::incrementCounter(Statistics::COUNTER_PACKAGES_SENT, 1);
    Statistics::incrementCounter(Statistics::COUNTER_HEADER_BYTES_SENT, RTP_HEADER_MIN_SIZE);
    Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_SENT, inputBufferByteSize);

    //no changes in buffer-size
    return inputBufferByteSize;
}

unsigned int ProcessorRTP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
    // unpack data from a rtp-package
    if (rtpPackage.get() == nullptr)
    {
        initPackageHandler(userData->maxBufferSize);
    }
    //read package from buffer
    auto result = rtpBuffer->readPackage(*rtpPackage);

    if (result == RTPBufferStatus::RTP_BUFFER_IS_PUFFERING)
    {
        std::cerr <<  "Buffer is buffering" << std::endl;
    }
    else if (result == RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW)
    {
        userData->isSilentPackage = true;
        std::cerr << "Output Buffer underflow" << std::endl;
    }
    else
    {
        userData->isSilentPackage = false;
    }

    const void* recvAudioData = rtpPackage->getRTPPackageData();
    unsigned int receivedPayloadSize = rtpPackage->getActualPayloadSize();
    memcpy(outputBuffer, recvAudioData, receivedPayloadSize);

    //set received payload size for all following processors to use
    return receivedPayloadSize;
}

bool ProcessorRTP::cleanUp()
{
    if(rtpPackage.get() == nullptr)
    {
        //if we never sent a RTP-package, there is no need to end the communication
        return true;
    }
    std::cout << "Communication terminated." << std::endl;
    return true;
}

void ProcessorRTP::initPackageHandler(unsigned int maxBufferSize)
{
    if(rtpPackage.get() == nullptr)
    {
        rtpPackage.reset(new RTPPackageHandler(maxBufferSize, payloadType));
    }
    participantDatabase[PARTICIPANT_SELF].ssrc = rtpPackage->ssrc;
    participantDatabase[PARTICIPANT_SELF].initialRTPTimestamp = rtpPackage->timestamp;
    participantDatabase[PARTICIPANT_SELF].extendedHighestSequenceNumber = rtpPackage->sequenceNr;
}
