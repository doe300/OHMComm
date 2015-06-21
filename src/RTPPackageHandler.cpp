/* 
 * File:   RTPPackage.cpp
 * Author: daniel
 * 
 * Created on May 02, 2015, 13:03 PM
 */

#include "RTPPackageHandler.h"

RTPPackageHandler::RTPPackageHandler(unsigned int sizeOfAudioData, PayloadType payloadType, unsigned int sizeOfRTPHeader)
{
	this->sizeOfAudioData = sizeOfAudioData;
	this->sizeOfRTPHeader = sizeOfRTPHeader;
	this->size = sizeOfAudioData + sizeOfRTPHeader;
	this->payloadType = payloadType;

	newRTPPackageBuffer = new char[size];
	workBuffer = new char[size];

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 tmp(seed1);
	this->randomGenerator = tmp;

	sequenceNr = getRandomNumber();
	timestamp = createStartingTimestamp();
	ssrc = getAudioSourceId();
}

auto RTPPackageHandler::getNewRTPPackage(void* audioData) -> void*
{
	RTPHeader RTPHeaderObject;

	RTPHeaderObject.version = 2;
	RTPHeaderObject.padding = 0;
	RTPHeaderObject.extension = 0;
	RTPHeaderObject.csrc_count = 0;
	RTPHeaderObject.marker = 0;
	RTPHeaderObject.payload_type = this->payloadType;
	RTPHeaderObject.sequence_number = (this->sequenceNr++) % UINT16_MAX;
        //we need steady clock so it will always change monotonically (etc. no change to/from daylight savings time)
        //additionally, we need to count with milliseconds precision
        //we add the random starting timestamp to meet the condition specified in the RTP standard
	RTPHeaderObject.timestamp = this->timestamp + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	RTPHeaderObject.ssrc = this->ssrc;

	// Copy RTPHeader and Audiodata in the buffer
	memcpy((char*)newRTPPackageBuffer, &RTPHeaderObject, this->sizeOfRTPHeader);
	memcpy((char*)(newRTPPackageBuffer)+sizeOfRTPHeader, audioData, this->sizeOfAudioData);

	return newRTPPackageBuffer;
}

auto RTPPackageHandler::getRTPPackageData(void *rtpPackage) -> void*
{
	if (rtpPackage == nullptr)
	{
		return (char*)workBuffer + sizeOfRTPHeader;
	}
	memcpy((char*)(workBuffer) + sizeOfRTPHeader, (char*)rtpPackage + sizeOfRTPHeader, sizeOfAudioData);
	return (char*)(workBuffer)+sizeOfRTPHeader;
}

auto RTPPackageHandler::getRTPPackageHeader(void *rtpPackage) -> RTPHeader*
{
	if (rtpPackage == nullptr)
	{
		return (RTPHeader*)workBuffer;
	}
	memcpy((char*)(workBuffer), (char*)rtpPackage, sizeOfRTPHeader);
	return (RTPHeader*)workBuffer;
}


unsigned int RTPPackageHandler::getRandomNumber()
{
	return this->randomGenerator();
}

unsigned int RTPPackageHandler::createStartingTimestamp()
{
    //start timestamp should be a random number
    return this->randomGenerator(); 
}

unsigned int RTPPackageHandler::getAudioSourceId()
{
    //SSRC should be a random number
    return this->randomGenerator();
}

auto RTPPackageHandler::getSize() -> unsigned int const
{
	return size;
}

auto RTPPackageHandler::getSizeOfRTPHeader() -> unsigned int const
{
	return sizeOfRTPHeader;
}

auto RTPPackageHandler::getSizeOfAudioData() -> unsigned int const
{
	return sizeOfAudioData;
}

auto RTPPackageHandler::getWorkBuffer() -> void*
{
	return this->workBuffer;
}

void RTPPackageHandler::createSilencePackage()
{
    RTPHeader silenceHeader;
    memcpy(workBuffer, &silenceHeader, sizeOfRTPHeader);
    memset((char *)(workBuffer) + sizeOfRTPHeader, 0, sizeOfAudioData);
}
