#include "ProcessorRTP.h"

ProcessorRTP::ProcessorRTP(std::string name, NetworkWrapper *networkwrapper, std::unique_ptr<RTPBuffer> *buffer) : AudioProcessor(name) 
{
	this->networkObject = networkwrapper;
        this->rtpBuffer = buffer;
}

void ProcessorRTP::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData)
{
	// pack data into a rtp-package
	if (rtpPackage == nullptr)
	{
		rtpPackage = new RTPPackage(inputBufferByteSize);
	}
	void* newRTPPackage = rtpPackage->getNewRTPPackage(inputBuffer, userData->streamTime);
	this->networkObject->sendDataNetworkWrapper(newRTPPackage, rtpPackage->getPacketSizeRTPPackage());
}

void ProcessorRTP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
	// unpack data from a rtp-package
	if (rtpPackage == nullptr)
	{
		rtpPackage = new RTPPackage(outputBufferByteSize);
	}
//	this->networkObject->recvDataNetworkWrapper(rtpPackage->getRecvBuffer(), rtpPackage->getPacketSizeRTPPackage());
        //read package from buffer
        (*rtpBuffer)->readPackage(*rtpPackage);
        void* recvAudioData = rtpPackage->getDataFromRTPPackage();
        memcpy(outputBuffer, recvAudioData, outputBufferByteSize);
}