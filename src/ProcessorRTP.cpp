#include "ProcessorRTP.h"
#include "Statistics.h"

ProcessorRTP::ProcessorRTP(std::string name, NetworkWrapper *networkwrapper, std::unique_ptr<RTPBuffer> *buffer) : AudioProcessor(name) 
{
	this->networkObject = networkwrapper;
        this->rtpBuffer = buffer;
}

unsigned int ProcessorRTP::getSupportedAudioFormats()
{
    return AudioConfiguration::AUDIO_FORMAT_ALL;
}

unsigned int ProcessorRTP::getSupportedSampleRates()
{
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

std::vector<int> ProcessorRTP::getSupportedBufferSizes(unsigned int sampleRate)
{
    return std::vector<int>({BUFFER_SIZE_ANY});
}

unsigned int ProcessorRTP::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData)
{
    // pack data into a rtp-package
    if (rtpPackage == nullptr)
    {
        rtpPackage = new RTPPackageHandler(userData->maxBufferSize);
    }
    void* newRTPPackage = rtpPackage->getNewRTPPackage(inputBuffer, inputBufferByteSize);
    //only send the number of bytes really required: header + actual payload-size
    this->networkObject->sendDataNetworkWrapper(newRTPPackage, rtpPackage->getRTPHeaderSize() + inputBufferByteSize);
    
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
    if (rtpPackage == nullptr)
    {
        rtpPackage = new RTPPackageHandler(userData->maxBufferSize);
    }
    //read package from buffer
    (*rtpBuffer)->readPackage(*rtpPackage);
    void* recvAudioData = rtpPackage->getRTPPackageData();
    unsigned int receivedPayloadSize = rtpPackage->getActualPayloadSize();
    memcpy(outputBuffer, recvAudioData, outputBufferByteSize);
    
    //set received payload size for all following processors to use
    return receivedPayloadSize;
}