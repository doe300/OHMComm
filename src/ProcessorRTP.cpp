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
	// Measure performance in ms
	//Statistics::TimerStartInputProcessing(this->getName());

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
    
	// Measure performance in ms
	//Statistics::TimerStopInputProcessing(this->getName());

    //no changes in buffer-size
    return inputBufferByteSize;
}

unsigned int ProcessorRTP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
	// Measure performance in ms
	//Statistics::TimerStartOutputProcessing(this->getName());

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
    
	// Measure performance in ms
	//Statistics::TimerStopOutputProcessing(this->getName());

    //set received payload size for all following processors to use
    return receivedPayloadSize;
}

bool ProcessorRTP::cleanUp()
{
	// Send a special packet, to tell the client that communication has been stopped
	char buffer = 255;
	this->networkObject->sendDataNetworkWrapper(&buffer, 1);
	return true;
}