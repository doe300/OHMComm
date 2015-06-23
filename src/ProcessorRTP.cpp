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
        rtpPackage = new RTPPackageHandler(inputBufferByteSize);
    }
    void* newRTPPackage = rtpPackage->getNewRTPPackage(inputBuffer);
    this->networkObject->sendDataNetworkWrapper(newRTPPackage, rtpPackage->getSize());
    
    //no changes in buffer-size
    return inputBufferByteSize;
}

unsigned int ProcessorRTP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
    // unpack data from a rtp-package
    if (rtpPackage == nullptr)
    {
        rtpPackage = new RTPPackageHandler(outputBufferByteSize);
    }
    //read package from buffer
    (*rtpBuffer)->readPackage(*rtpPackage);
    void* recvAudioData = rtpPackage->getRTPPackageData();
    memcpy(outputBuffer, recvAudioData, outputBufferByteSize);
    
    //no changes in buffer-size
    return outputBufferByteSize;
}