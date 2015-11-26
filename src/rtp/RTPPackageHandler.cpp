/* 
 * File:   RTPPackage.cpp
 * Author: daniel
 * 
 * Created on May 02, 2015, 13:03 PM
 */

#include "rtp/RTPPackageHandler.h"

RTPPackageHandler::RTPPackageHandler(unsigned int maximumPayloadSize, PayloadType payloadType)
{
    this->maximumPayloadSize = maximumPayloadSize;
    this->maximumBufferSize = maximumPayloadSize + RTP_HEADER_MAX_SIZE;
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

    newRTPHeader.setPayloadType(payloadType);
    newRTPHeader.setSequenceNumber((this->sequenceNr++) % UINT16_MAX);
    //we need steady clock so it will always change monotonically (etc. no change to/from daylight savings time)
    //additionally, we need to count with milliseconds precision
    //we add the random starting timestamp to meet the condition specified in the RTP standard
    newRTPHeader.setTimestamp(getCurrentRTPTimestamp());
    newRTPHeader.setSSRC(this->ssrc);

    // Copy RTPHeader and Audiodata in the buffer
    memcpy((char*)workBuffer, &newRTPHeader, RTP_HEADER_MIN_SIZE);
    memcpy((char*)(workBuffer)+ RTP_HEADER_MIN_SIZE, audioData, payloadSize);
    actualPayloadSize = payloadSize;

    return workBuffer;
}

const void* RTPPackageHandler::getRTPPackageData() const
{
    return (char*)(workBuffer) + getRTPHeaderSize() + getRTPHeaderExtensionSize();
}

const RTPHeader* RTPPackageHandler::getRTPPackageHeader() const
{
    return (RTPHeader*)workBuffer;
}

//TODO needs testing
const RTPHeaderExtension RTPPackageHandler::getRTPHeaderExtension() const
{
    if(!getRTPPackageHeader()->hasExtension())
    {
        //return empty extension
        return RTPHeaderExtension(0);
    }
    //we must copy the contents of the header-extension, because we don't have any dynamic-sized array
    const RTPHeaderExtension* readEx = (RTPHeaderExtension*)((char*)(workBuffer) + getRTPHeaderSize());
    RTPHeaderExtension ex(readEx->getLength());
    ex.setProfile(readEx->getProfile());
    memcpy(ex.getExtension(), ((char*)(workBuffer) + getRTPHeaderSize() + RTP_HEADER_EXTENSION_MIN_SIZE), readEx->getLength());
    return ex;
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

unsigned int RTPPackageHandler::getRTPHeaderSize() const
{
    return RTP_HEADER_MIN_SIZE + getRTPPackageHeader()->getCSRCCount() * sizeof(uint32_t);
}

unsigned int RTPPackageHandler::getRTPHeaderExtensionSize() const
{
    if(((RTPHeader*)workBuffer)->hasExtension())
    {
        const RTPHeaderExtension* readEx = (RTPHeaderExtension*)((char*)(workBuffer) + getRTPHeaderSize());
        return RTP_HEADER_EXTENSION_MIN_SIZE + readEx->getLength() * sizeof(uint32_t);
    }
    else
    {
        return 0;
    }
}


unsigned int RTPPackageHandler::getMaximumPackageSize() const
{
    return maximumBufferSize;
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
    RTPHeader silenceHeader;
    memcpy(workBuffer, &silenceHeader, RTP_HEADER_MIN_SIZE);
    memset((char *)(workBuffer) + RTP_HEADER_MIN_SIZE, 0, maximumPayloadSize);
}

unsigned int RTPPackageHandler::getSSRC() const
{
    return ssrc;
}

uint32_t RTPPackageHandler::getCurrentRTPTimestamp() const
{
    return timestamp + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
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
    if(readHeader->getVersion() != RTPHeader::VERSION)
    {
        //version is always 2 per specification
        return false;
    }
    //we have no more fields to eliminate the package as RTP-package, so we accept it
    return true;
}


