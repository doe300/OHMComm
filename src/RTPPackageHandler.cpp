/* 
 * File:   RTPPackage.cpp
 * Author: daniel
 * 
 * Created on May 02, 2015, 13:03 PM
 */

#include "RTPPackageHandler.h"

RTPPackageHandler::RTPPackageHandler(unsigned int maximumPayloadSize, PayloadType payloadType, unsigned int sizeOfRTPHeader)
{
	this->maximumPayloadSize = maximumPayloadSize;
	this->sizeOfRTPHeader = sizeOfRTPHeader;
	this->maximumBufferSize = maximumPayloadSize + sizeOfRTPHeader;
	this->payloadType = payloadType;

	workBuffer = new char[maximumBufferSize];

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 tmp(seed1);
	this->randomGenerator = tmp;

	sequenceNr = getRandomNumber();
	timestamp = createStartingTimestamp();
	ssrc = getAudioSourceId();
}

RTPPackageHandler::~RTPPackageHandler()
{
    delete[] (char*)workBuffer;
}

const void* RTPPackageHandler::createNewRTPPackage(const void* audioData, unsigned int payloadSize)
{
	RTPHeader newRTPHeader;

	newRTPHeader.version = 2;
	newRTPHeader.padding = 0;
	newRTPHeader.extension = 0;
	newRTPHeader.csrc_count = 0;
	newRTPHeader.marker = 0;
	newRTPHeader.payload_type = this->payloadType;
	newRTPHeader.sequence_number = (this->sequenceNr++) % UINT16_MAX;
        //we need steady clock so it will always change monotonically (etc. no change to/from daylight savings time)
        //additionally, we need to count with milliseconds precision
        //we add the random starting timestamp to meet the condition specified in the RTP standard
	newRTPHeader.timestamp = this->timestamp + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	newRTPHeader.ssrc = this->ssrc;

	// Copy RTPHeader and Audiodata in the buffer
	memcpy((char*)workBuffer, &newRTPHeader, this->sizeOfRTPHeader);
	memcpy((char*)(workBuffer)+sizeOfRTPHeader, audioData, payloadSize);
        actualPayloadSize = payloadSize;

	return workBuffer;
}

const void* RTPPackageHandler::getRTPPackageData() const
{
    return (char*)(workBuffer) + sizeOfRTPHeader;
}

const RTPHeader* RTPPackageHandler::getRTPPackageHeader() const
{
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

unsigned int RTPPackageHandler::getMaximumPackageSize() const
{
    return maximumBufferSize;
}

unsigned int RTPPackageHandler::getRTPHeaderSize() const
{
    return sizeOfRTPHeader;
}

unsigned int RTPPackageHandler::getMaximumPayloadSize() const
{
    return maximumPayloadSize;
}

unsigned int RTPPackageHandler::getActualPayloadSize() const
{
    return actualPayloadSize;
}

void RTPPackageHandler::setActualPayloadSize(unsigned int payloadSize)
{
    this->actualPayloadSize = payloadSize;
}

void* RTPPackageHandler::getWorkBuffer()
{
    return this->workBuffer;
}

void RTPPackageHandler::createSilencePackage()
{
    RTPHeader silenceHeader{0};
    memcpy(workBuffer, &silenceHeader, sizeOfRTPHeader);
    memset((char *)(workBuffer) + sizeOfRTPHeader, 0, maximumPayloadSize);
}

unsigned int RTPPackageHandler::getSSRC() const
{
    return ssrc;
}

bool RTPPackageHandler::isRTPPackage(const void* packageBuffer, unsigned int packageLength)
{
    //1. check for package size, if large enough
    if(packageLength < RTP_HEADER_MIN_SIZE)
    {
        return false;
    }
    //2. we assume, it is an RTP package
    const RTPHeader* readHeader = (const RTPHeader* )packageBuffer;

    //3. check for header-fields
    if(readHeader->version != 2)
    {
        //version is always 2 per specification
        return false;
    }
    //we have no more fields to eliminate the package as RTP-package, so we accept it
    return true;
}


