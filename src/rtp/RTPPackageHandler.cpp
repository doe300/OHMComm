/* 
 * File:   RTPPackage.cpp
 * Author: daniel
 * 
 * Created on May 02, 2015, 13:03 PM
 */

#include "rtp/RTPPackageHandler.h"

using namespace ohmcomm::rtp;

RTPPackageHandler::RTPPackageHandler(unsigned int maximumPayloadSize, Participant& ourselves) : ourselves(ourselves),
        maximumPayloadSize(maximumPayloadSize), maximumBufferSize(maximumPayloadSize + RTPHeader::MAX_HEADER_SIZE), workBuffer(maximumBufferSize)
{
    sequenceNr = Utility::randomNumber();
    initialTimestamp = Utility::randomNumber();
}

RTPPackageHandler::~RTPPackageHandler()
{
    
}

const void* RTPPackageHandler::createNewRTPPackage(const void* audioData, unsigned int payloadSize)
{
    RTPHeader newRTPHeader;

    newRTPHeader.setPayloadType((PayloadType)ourselves.payloadType);
    newRTPHeader.setSequenceNumber((this->sequenceNr++) % UINT16_MAX);
    newRTPHeader.setTimestamp(getCurrentRTPTimestamp());
    newRTPHeader.setSSRC(ourselves.ssrc);

    // Copy RTPHeader and audio-data in the buffer
    memcpy(workBuffer.data(), &newRTPHeader, RTPHeader::MIN_HEADER_SIZE);
    memcpy(workBuffer.data() + RTPHeader::MIN_HEADER_SIZE, audioData, payloadSize);
    actualPayloadSize = payloadSize;

    return workBuffer.data();
}

const void* RTPPackageHandler::getRTPPackageData() const
{
    return getReadBuffer() + getRTPHeaderSize() + getRTPHeaderExtensionSize();
}

const RTPHeader* RTPPackageHandler::getRTPPackageHeader() const
{
    return (RTPHeader*)getReadBuffer();
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
    const RTPHeaderExtension* readEx = (RTPHeaderExtension*)(getReadBuffer() + getRTPHeaderSize());
    RTPHeaderExtension ex(readEx->getLength());
    ex.setProfile(readEx->getProfile());
    memcpy(ex.getExtension(), (getReadBuffer() + getRTPHeaderSize() + RTPHeaderExtension::MIN_EXTENSION_SIZE), readEx->getLength());
    return ex;
}

unsigned int RTPPackageHandler::getRTPHeaderSize() const
{
    return RTPHeader::MIN_HEADER_SIZE + getRTPPackageHeader()->getCSRCCount() * sizeof(uint32_t);
}

unsigned int RTPPackageHandler::getRTPHeaderExtensionSize() const
{
    if(((RTPHeader*)getReadBuffer())->hasExtension())
    {
        const RTPHeaderExtension* readEx = (RTPHeaderExtension*)(getReadBuffer() + getRTPHeaderSize());
        return RTPHeaderExtension::MIN_EXTENSION_SIZE + readEx->getLength() * sizeof(uint32_t);
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

char* RTPPackageHandler::getWriteBuffer(const unsigned int minSize)
{
    //make sure, buffer is large enough
    workBuffer.reserve(minSize);
    return workBuffer.data();
}

const char* RTPPackageHandler::getReadBuffer() const
{
    return workBuffer.data();
}

void RTPPackageHandler::createSilencePackage()
{
    RTPHeader silenceHeader;
    memcpy(workBuffer.data(), &silenceHeader, RTPHeader::MIN_HEADER_SIZE);
    memset(workBuffer.data() + RTPHeader::MIN_HEADER_SIZE, 0, maximumPayloadSize);
    actualPayloadSize = maximumPayloadSize;
}

uint32_t RTPPackageHandler::getCurrentRTPTimestamp() const
{
    //we need steady clock so it will always change monotonically (etc. no change to/from daylight savings time)
    //additionally, we need to count with milliseconds precision
    //we add the random starting timestamp to meet the condition specified in the RTP standard
    return initialTimestamp + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool RTPPackageHandler::isRTPPackage(const void* packageBuffer, unsigned int packageLength)
{
    //1. check for package size, if large enough
    if(packageLength < RTPHeader::MIN_HEADER_SIZE)
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

unsigned int RTPPackageHandler::getInitialTimestamp() const
{
    return initialTimestamp;
}

unsigned int RTPPackageHandler::getCurrentSequenceNumber() const
{
    return sequenceNr;
}

