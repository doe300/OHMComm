#include "ProcessorRTP.h"

ProcessorRTP::ProcessorRTP(std::string name, NetworkWrapper *networkwrapper) : AudioProcessor(name) 
{
	this->networkObject = networkwrapper;
}

void ProcessorRTP::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, void *userData)
{
	// pack data into a rtp-package
	if (rtpPackage == NULL)
	{
		rtpPackage = new RTPPackage(inputBufferByteSize);
	}
	void* newRTPPackage = rtpPackage->getNewRTPPackage(inputBuffer);
	this->networkObject->sendDataNetworkWrapper(newRTPPackage, rtpPackage->getPacketSizeRTPPackage());
}

void ProcessorRTP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, void *userData)
{
	// unpack data from a rtp-package
	if (rtpPackage == NULL)
	{
		rtpPackage = new RTPPackage(outputBufferByteSize);
	}
	this->networkObject->recvDataNetworkWrapper(rtpPackage->getRecvBuffer(), rtpPackage->getPacketSizeRTPPackage());
	void* recvAudioData = rtpPackage->getDataFromRTPPackage();
	memcpy(outputBuffer, recvAudioData, outputBufferByteSize);
}