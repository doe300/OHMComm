#include "ProcessorRTP.h"

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