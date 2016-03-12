/*
 * File:   RTCPPackageHandler.cpp
 * Author: daniel
 *
 * Created on June 16, 2015, 5:47 PM
 */

#include <stdexcept>

#include "rtp/RTCPPackageHandler.h"

using namespace ohmcomm::rtp;

RTCPPackageHandler::RTCPPackageHandler() : rtcpPackageBuffer(8000)
{
    //maximal SR size: 8 (header) + 20 (sender-info) + 31 (5 bit RC count) * 24 (reception report) = 772
    //maximal RR size: 8 (header) + 31 (5 bit RC count) * 24 (reception report) = 752
    //maximal SDES size: 8 (header) + 31 (5 bit SDES count) * (1 (SDES type) + 1 (SDES length) + 255 (max 255 characters)) = 7975
    //maximal BYE size: 8 (header) + 1 (length) + 255 (max 255 characters) = 264
}

RTCPPackageHandler::~RTCPPackageHandler()
{
}


const void* RTCPPackageHandler::createSenderReportPackage(RTCPHeader& header, const SenderInformation& senderInfo, const std::vector<ReceptionReport>& reports, const unsigned int offset)
{
    char* bufferStart = rtcpPackageBuffer.data() + offset;
    //adjust header
    header.setType(RTCP_PACKAGE_SENDER_REPORT);
    header.setCount(reports.size());
    header.setLength(calculateLengthField(RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE));
    assertCapacity(offset + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);

    memcpy(bufferStart, &header, RTCP_HEADER_SIZE);
    memcpy(bufferStart + RTCP_HEADER_SIZE, &senderInfo, RTCP_SENDER_INFO_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(bufferStart + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return bufferStart;
}

const void* RTCPPackageHandler::createReceiverReportPackage(RTCPHeader& header, const std::vector<ReceptionReport>& reports, const unsigned int offset)
{
    char* bufferStart = rtcpPackageBuffer.data() + offset;
    //adjust header
    header.setType(RTCP_PACKAGE_RECEIVER_REPORT);
    header.setCount(reports.size());
    header.setLength(calculateLengthField(RTCP_HEADER_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE));
    assertCapacity(offset + RTCP_HEADER_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);

    memcpy(bufferStart, &header, RTCP_HEADER_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(bufferStart + RTCP_HEADER_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return bufferStart;
}

const void* RTCPPackageHandler::createSourceDescriptionPackage(RTCPHeader& header, const std::vector<SourceDescription>& descriptions, const unsigned int bufferOffset)
{
    char* bufferStart = rtcpPackageBuffer.data() + bufferOffset;
    //adjust header
    header.setType(RTCP_PACKAGE_SOURCE_DESCRIPTION);
    //we only carry a SDES for one SSRC
    header.setCount(1);

    uint16_t offset = RTCP_HEADER_SIZE;
    for(unsigned int i = 0; i < descriptions.size(); i++)
    {
        assertCapacity(bufferOffset + offset + 32);
        //set type
        memcpy(bufferStart + offset, &descriptions[i].type, 1);
        offset++;
        //set length
        uint8_t size = descriptions[i].value.size();
        memcpy(bufferStart + offset, &size, 1);
        offset++;
        //set value
        memcpy(bufferStart + offset, descriptions[i].value.c_str(), size);
        offset+=size;
    }

    //padding (!= the header padding-flag) to the next multiple of 4 bytes
    uint8_t padding = 4 - ((1 + offset) % 4);
    offset+= padding;
    if(padding != 0)
    {
        //RFC 3550 specifies a list of SDES items to be padded until the next 32bit block
        //apply padding - fill with zero-bytes
        memset(bufferStart + offset, RTCP_SOURCE_END, padding);
    }

    //now offset is the same as the payload length
    header.setLength(calculateLengthField(offset));
    
    //we need to copy the header last, because of the length-field
    memcpy(bufferStart, &header, RTCP_HEADER_SIZE);
    return bufferStart;
}

const void* RTCPPackageHandler::createByePackage(RTCPHeader& header, const std::string& byeMessage, const unsigned int offset)
{
    char* bufferStart = rtcpPackageBuffer.data() + offset;
    //adjust header
    header.setType(RTCP_PACKAGE_GOODBYE);
    //we have one source, our SSRC
    header.setCount(1);
    
    uint8_t length = byeMessage.size();
    //header + length + text + padding
    header.setLength(calculateLengthField(RTCP_HEADER_SIZE + 1 + length + 1));
    assertCapacity(offset + RTCP_HEADER_SIZE + 1 + length + 1);

    memcpy(bufferStart, &header, RTCP_HEADER_SIZE);
    memcpy(bufferStart + RTCP_HEADER_SIZE, &length, 1);
    if(length > 0)
    {
        memcpy(bufferStart + RTCP_HEADER_SIZE + 1, byeMessage.c_str(), length);
        
        //padding to the next multiple of 4 bytes - this is not the RTCP header padding flag!
        uint8_t padding = 4 - ((1 + length) % 4);
        if(padding != 0)
        {
            //apply padding - fill with zeroes
            memset(bufferStart + RTCP_HEADER_SIZE + 1 + length, 0, padding);
        }
    }
    return bufferStart;
}

const void* RTCPPackageHandler::createApplicationDefinedPackage(RTCPHeader& header, ApplicationDefined& appDefined, const unsigned int offset)
{
    char* bufferStart = rtcpPackageBuffer.data() + offset;
    //adjust header
    header.setType(RTCP_PACKAGE_APPLICATION_DEFINED);
    //the length is the header-size, the data-length and 4 bytes for the app-defined name
    header.setLength(calculateLengthField(RTCP_HEADER_SIZE + sizeof(appDefined.name) + appDefined.dataLength));
    assertCapacity(offset + RTCP_HEADER_SIZE + sizeof(appDefined.name) + appDefined.dataLength);
    //application defined data must be already padded to 32bit
    if((appDefined.dataLength % 4) != 0)
    {
        throw std::invalid_argument("Length of Application Defined data must be a multiple of 32 bits (4 bytes)!");
    }
    header.setPadding(0);
    header.setCount(appDefined.subType);

    memcpy(bufferStart, &header, RTCP_HEADER_SIZE);
    memcpy(bufferStart + RTCP_HEADER_SIZE, appDefined.name, sizeof(appDefined.name));
    if(appDefined.dataLength > 0)
    {
        memcpy(bufferStart + RTCP_HEADER_SIZE + sizeof(appDefined.name), appDefined.data, appDefined.dataLength);
    }
    return bufferStart;
}


std::vector<ReceptionReport> RTCPPackageHandler::readSenderReport(const void* senderReportPackage, uint16_t packageLength, RTCPHeader& header, SenderInformation& senderInfo) const
{
    RTCPHeader *readHeader = (RTCPHeader *)senderReportPackage;
    //copy header to out-parameter
    header = *readHeader;

    char* startPtr = (char*)senderReportPackage + RTCP_HEADER_SIZE;
    SenderInformation *readInfo = (SenderInformation*)startPtr;
    //copy sender-info to out-parameter
    senderInfo = *readInfo;

    startPtr += RTCP_SENDER_INFO_SIZE;
    std::vector<ReceptionReport> reports(header.getCount());
    for(unsigned int i = 0; i < header.getCount(); i++)
    {
        ReceptionReport *readReport = (ReceptionReport *)startPtr;
        reports[i] = *readReport;
        startPtr += RTCP_RECEPTION_REPORT_SIZE;
    }
    return reports;
}

std::vector<ReceptionReport> RTCPPackageHandler::readReceiverReport(const void* receiverReportPackage, uint16_t packageLength, RTCPHeader& header) const
{
    RTCPHeader *readHeader = (RTCPHeader *)receiverReportPackage;
    //copy header to out-parameter
    header = *readHeader;

    std::vector<ReceptionReport> reports(header.getCount());
    char* startPtr = (char*)receiverReportPackage + RTCP_HEADER_SIZE;
    for(unsigned int i = 0; i < header.getCount(); i++)
    {
        startPtr += i * RTCP_RECEPTION_REPORT_SIZE;
        ReceptionReport *readReport = (ReceptionReport *)startPtr;
        reports[i] = *readReport;
    }
    return reports;
}

std::vector<SourceDescription> RTCPPackageHandler::readSourceDescription(const void* sourceDescriptionPackage, uint16_t packageLength, RTCPHeader& header) const
{
    RTCPHeader *readHeader = (RTCPHeader *)sourceDescriptionPackage;
    //copy header to out-parameter
    header = *readHeader;

    std::vector<SourceDescription> descriptions;
    descriptions.reserve(8);
    uint16_t offset = RTCP_HEADER_SIZE;
    //we don't know how many SDES items there are for a SSRC
    while(offset < packageLength)
    {
        RTCPSourceDescriptionType *type = (RTCPSourceDescriptionType*)sourceDescriptionPackage + offset;
        if(*type == RTCP_SOURCE_END)
        {
            //RFC 3550 specifies, the SDES items are trailed by zeros, which are to be tested for type == 0
            break;
        }
        offset++;
        uint8_t *valueLength = (uint8_t *)sourceDescriptionPackage + offset;
        offset++;
        char *value = (char*)sourceDescriptionPackage + offset;
        descriptions.push_back({*type, std::string(value, *valueLength)});
        offset += *valueLength;
    }
    
    //here would be reading SDES of the CSRCs
    
    return descriptions;
}


std::string RTCPPackageHandler::readByeMessage(const void* byePackage, uint16_t packageLength, RTCPHeader& header) const
{
    RTCPHeader *readHeader = (RTCPHeader *)byePackage;
    //copy header to out-parameter
    header = *readHeader;
    //skip the header and all SSRC/CSRCs - the SSRC is part of the header and counted in #count, so don't skip it twice
    unsigned int offset = RTCP_HEADER_SIZE + (readHeader->getCount() - 1) * sizeof(uint32_t);
    uint8_t length = *((char*)byePackage + offset);

    const char *byeMessage = (const char*)byePackage + offset + 1;
    return std::string(byeMessage, length);
}

ApplicationDefined RTCPPackageHandler::readApplicationDefinedMessage(const void* appDefinedPackage, uint16_t packageLength, RTCPHeader& header) const
{
    RTCPHeader *readHeader = (RTCPHeader *)appDefinedPackage;
    //copy header to out-parameter
    header = *readHeader;

    char name[4];
    memcpy(name, (char*)appDefinedPackage + RTCP_HEADER_SIZE, sizeof(name));
    //data-length = length of whole package - header - name
    uint16_t dataLength = getRTCPPackageLength(readHeader->getLength()) - RTCP_HEADER_SIZE - sizeof(name);
    char* data = (char*)appDefinedPackage + RTCP_HEADER_SIZE + sizeof(name);

    ApplicationDefined result(name, dataLength, data, readHeader->getCount());
    return result;
}

RTCPHeader RTCPPackageHandler::readRTCPHeader(const void* rtcpPackage, unsigned int packageLength) const
{
    RTCPHeader *readHeader = (RTCPHeader *)rtcpPackage;
    RTCPHeader headerCopy = *readHeader;
    return headerCopy;
}

bool RTCPPackageHandler::isRTCPPackage(const void* packageBuffer, const unsigned int packageLength)
{
    //1. check for package size, if large enough
    if(packageLength < RTCP_HEADER_SIZE)
    {
        return false;
    }
    //2. we assume, it is an RTCP package
    RTCPHeader *readHeader = (RTCPHeader* )packageBuffer;

    //3. check for header-fields
    if(readHeader->getVersion() != 2)
    {
        //version is always 2 per specification
        return false;
    }
    switch(readHeader->getType())
    {
        //the packageType must be one of those values per specification
        //this comparison makes sure, RTP-packages are not accepted, because of the different bit-pattern in the headers
        case RTCP_PACKAGE_SENDER_REPORT:
        case RTCP_PACKAGE_RECEIVER_REPORT:
        case RTCP_PACKAGE_SOURCE_DESCRIPTION:
        case RTCP_PACKAGE_APPLICATION_DEFINED:
        case RTCP_PACKAGE_GOODBYE:
            break;
        default:
            return false;
    }
    //4. check if the complete package fits in the buffer
    //the length is the number of 32-bit groups
    if(getRTCPPackageLength(readHeader->getLength()-1) > packageLength)
    {
        return false;
    }
    //we have no more fields to eliminate the package as RTCP-package, so we accept it
    return true;
}

unsigned int RTCPPackageHandler::getRTCPPackageLength(unsigned int lengthHeaderField)
{
    //"The length of this RTCP packet in 32-bit words minus one, including the header and any padding"
    return (lengthHeaderField + 1) * 4;
}

unsigned int RTCPPackageHandler::getRTCPCompoundPackagesCount(const void* rtcpCompoundBuffer, const unsigned int maxPackageLength)
{
    unsigned int remainingPackageLength = maxPackageLength;
    unsigned int numPackages = 0;
    const void* packagePointer = rtcpCompoundBuffer;
    //we iterate through the buffer checking for RTCP packages until the test fails
    while(isRTCPPackage(packagePointer, remainingPackageLength))
    {
        ++numPackages;
        unsigned int currentPackageSize = getRTCPPackageLength(((const RTCPHeader*)packagePointer)->getLength());
        remainingPackageLength -= currentPackageSize;
        packagePointer = (char*)packagePointer + currentPackageSize;
    }
    return numPackages;
}

uint8_t RTCPPackageHandler::calculateLengthField(uint16_t length)
{
    //4 byte = 32 bit
    //we need to always round up, so we add 3:
    //e.g. 16 / 4 = 4 <-> (16+3) / 4 = 4
    //17 / 4 = 4 => (17+3) / 4 = 5!
    return ((length+3) / 4) - 1;
}

void RTCPPackageHandler::assertCapacity(unsigned int newCapacity)
{
    if(rtcpPackageBuffer.capacity() < newCapacity)
    {
        //should never be necessary, but just to be sure...
        rtcpPackageBuffer.resize(newCapacity);
    }
}
