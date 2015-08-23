#include "ProcessorRTP.h"
#include "Statistics.h"
#include "RTCPPackageHandler.h"

ProcessorRTP::ProcessorRTP(std::string name, std::shared_ptr<NetworkWrapper> networkwrapper, std::shared_ptr<RTPBufferHandler> buffer) : AudioProcessor(name)
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
    this->networkObject->sendData(newRTPPackage, rtpPackage->getRTPHeaderSize() + inputBufferByteSize);

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
    auto result = rtpBuffer->readPackage(*rtpPackage);

	if (result == RTP_BUFFER_IS_PUFFERING)
	{
		std::cerr <<  "Buffer is puffering" << std::endl;
	}
	else if (result == RTP_BUFFER_OUTPUT_UNDERFLOW)
	{
		std::cerr << "Output Buffer underflow" << std::endl;
	}

    void* recvAudioData = rtpPackage->getRTPPackageData();
    unsigned int receivedPayloadSize = rtpPackage->getActualPayloadSize();
    memcpy(outputBuffer, recvAudioData, outputBufferByteSize);

    //set received payload size for all following processors to use
    return receivedPayloadSize;
}

bool ProcessorRTP::cleanUp()
{
    if(rtpPackage == nullptr)
    {
        //if we never sent a RTP-package, there is no need to end the communication
        return true;
    }
    // Send a RTCP BYE-packet, to tell the other side that communication has been stopped
    RTCPPackageHandler rtcpHandler;
    RTCPHeader byeHeader(rtpPackage->getSSRC());
    void* packageBuffer = rtcpHandler.createByePackage(byeHeader, "Program exit");
    this->networkObject->sendData(packageBuffer, RTCPPackageHandler::getRTCPPackageLength(byeHeader.length));
    std::cout << "Communication terminated." << std::endl;
    //clean up send-buffer
    delete rtpPackage;
    rtpPackage = nullptr;
    return true;
}
