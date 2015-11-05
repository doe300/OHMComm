/*
 * File:   RTCPPackageHandler.cpp
 * Author: daniel
 *
 * Created on June 16, 2015, 5:47 PM
 */

#include <stdexcept>

#include "RTCPPackageHandler.h"

RTCPPackageHandler::RTCPPackageHandler() : maxPackageSize(8000)
{
    //maximal SR size: 8 (header) + 20 (sender-info) + 31 (5 bit RC count) * 24 (reception report) = 772
    //maximal RR size: 8 (header) + 31 (5 bit RC count) * 24 (reception report) = 752
    //maximal SDES size: 8 (header) + 31 (5 bit SDES count) * (1 (SDES type) + 1 (SDES length) + 255 (max 255 characters)) = 7975
    //maximal BYE size: 8 (header) + 1 (length) + 255 (max 255 characters) = 264
    rtcpPackageBuffer = new char[maxPackageSize];
}

RTCPPackageHandler::~RTCPPackageHandler()
{
    delete[] rtcpPackageBuffer;
}


void* RTCPPackageHandler::createSenderReportPackage(RTCPHeader& header, SenderInformation& senderInfo, const std::vector<ReceptionReport>& reports)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_SENDER_REPORT;
    header.receptionReportOrSourceCount = reports.size();
    header.length = calculateLengthField(RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);

    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE, &senderInfo, RTCP_SENDER_INFO_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createReceiverReportPackage(RTCPHeader& header, const std::vector<ReceptionReport>& reports)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_RECEIVER_REPORT;
    header.receptionReportOrSourceCount = reports.size();
    header.length = calculateLengthField(RTCP_HEADER_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);

    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createSourceDescriptionPackage(RTCPHeader& header, const std::vector<SourceDescription>& descriptions)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_SOURCE_DESCRIPTION;
    header.receptionReportOrSourceCount = descriptions.size();

    uint16_t offset = RTCP_HEADER_SIZE;
    for(unsigned int i = 0; i < descriptions.size(); i++)
    {
        //set type
        memcpy(rtcpPackageBuffer + offset, &descriptions[i].type, 1);
        offset++;
        //set length
        uint8_t size = descriptions[i].value.size();
        memcpy(rtcpPackageBuffer + offset, &size, 1);
        offset++;
        //set value
        memcpy(rtcpPackageBuffer + offset, descriptions[i].value.c_str(), size);
        offset+=size;
    }
    //now offset is the same as the payload length
    //padding to the next multiple of 4 bytes
    uint8_t padding = 4 - ((1 + offset) % 4);
    header.length = calculateLengthField(offset);
    header.padding = padding != 0;

     if(padding != 0)
    {
        //apply padding - fill with number of padded bytes
        memset(rtcpPackageBuffer + offset, padding, padding);
    }

    //we need to copy the header last, because of the length- and padding-fields
    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createByePackage(RTCPHeader& header, const std::string& byeMessage)
{
    header.packageType = RTCP_PACKAGE_GOODBYE;
    uint8_t length = byeMessage.size();
    //padding to the next multiple of 4 bytes
    uint8_t padding = 4 - ((1 + length) % 4);
    header.length = calculateLengthField(RTCP_HEADER_SIZE + 1 + length);
    header.padding = padding != 0;

    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE, &length, 1);
    if(length > 0)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + 1, byeMessage.c_str(), length);
    }
    if(padding != 0)
    {
        //apply padding - fill with number of padded bytes
        memset(rtcpPackageBuffer + RTCP_HEADER_SIZE + 1 + length, padding, padding);
    }
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createApplicationDefinedPackage(RTCPHeader& header, ApplicationDefined& appDefined)
{
    header.packageType = RTCP_PACKAGE_APPLICATION_DEFINED;
    //the length is the header-size, the data-length and 4 bytes for the app-defined name
    header.length = calculateLengthField(RTCP_HEADER_SIZE + sizeof(appDefined.name) + appDefined.dataLength);
    //application defined data must be already padded to 32bit
    if((appDefined.dataLength % 4) != 0)
    {
        throw std::invalid_argument("Length of Application Defined data must be a multiple of 32 bits (4 bytes)!");
    }
    header.padding = 0;
    header.receptionReportOrSourceCount = appDefined.subType;

    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE, appDefined.name, sizeof(appDefined.name));
    if(appDefined.dataLength > 0)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + sizeof(appDefined.name), appDefined.data, appDefined.dataLength);
    }
    return rtcpPackageBuffer;
}


std::vector<ReceptionReport> RTCPPackageHandler::readSenderReport(void* senderReportPackage, uint16_t packageLength, RTCPHeader& header, SenderInformation& senderInfo)
{
    RTCPHeader *readHeader = (RTCPHeader *)senderReportPackage;
    //copy header to out-parameter
    header = *readHeader;

    SenderInformation *readInfo = (SenderInformation*)((char *)senderReportPackage + RTCP_HEADER_SIZE);
    //copy sender-info to out-parameter
    senderInfo = *readInfo;

    std::vector<ReceptionReport> reports(header.receptionReportOrSourceCount);
    for(unsigned int i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        ReceptionReport *readReport = (ReceptionReport *)senderReportPackage + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + i * RTCP_RECEPTION_REPORT_SIZE;
        reports[i] = *readReport;
    }
    return reports;
}

std::vector<ReceptionReport> RTCPPackageHandler::readReceiverReport(void* receiverReportPackage, uint16_t packageLength, RTCPHeader& header)
{
    RTCPHeader *readHeader = (RTCPHeader *)receiverReportPackage;
    //copy header to out-parameter
    header = *readHeader;

    std::vector<ReceptionReport> reports(header.receptionReportOrSourceCount);
    char* startPtr = (char*)receiverReportPackage + RTCP_HEADER_SIZE;
    for(unsigned int i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        startPtr += i * RTCP_RECEPTION_REPORT_SIZE;
        ReceptionReport *readReport = (ReceptionReport *)startPtr;
        reports[i] = *readReport;
    }
    return reports;
}

std::vector<SourceDescription> RTCPPackageHandler::readSourceDescription(void* sourceDescriptionPackage, uint16_t packageLength, RTCPHeader& header)
{
    RTCPHeader *readHeader = (RTCPHeader *)sourceDescriptionPackage;
    //copy header to out-parameter
    header = *readHeader;

    std::vector<SourceDescription> descriptions(header.receptionReportOrSourceCount);
    uint16_t offset = RTCP_HEADER_SIZE;
    for(uint8_t i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        RTCPSourceDescriptionType *type = (RTCPSourceDescriptionType*)sourceDescriptionPackage + offset;
        offset++;
        uint8_t *valueLength = (uint8_t *)sourceDescriptionPackage + offset;
        offset++;
        char *value = (char*)sourceDescriptionPackage + offset;
        descriptions[i].type = *type;
        descriptions[i].value = std::string(value, *valueLength);
        offset += *valueLength;
    }
    return descriptions;
}


std::string RTCPPackageHandler::readByeMessage(void* byePackage, uint16_t packageLength, RTCPHeader& header)
{
    RTCPHeader *readHeader = (RTCPHeader *)byePackage;
    //copy header to out-parameter
    header = *readHeader;
    uint8_t length = *((char*)byePackage + RTCP_HEADER_SIZE);

    const char *byeMessage = (const char*)byePackage + RTCP_HEADER_SIZE + 1;
    return std::string(byeMessage, length);
}

ApplicationDefined RTCPPackageHandler::readApplicationDefinedMessage(void* appDefinedPackage, uint16_t packageLength, RTCPHeader& header)
{
    RTCPHeader *readHeader = (RTCPHeader *)appDefinedPackage;
    //copy header to out-parameter
    header = *readHeader;

    char name[4];
    memcpy(name, (char*)appDefinedPackage + RTCP_HEADER_SIZE, sizeof(name));
    //data-length = length of whole package - header - name
    uint16_t dataLength = getRTCPPackageLength(readHeader->length) - RTCP_HEADER_SIZE - sizeof(name);
    char* data = (char*)appDefinedPackage + RTCP_HEADER_SIZE + sizeof(name);

    ApplicationDefined result(name, dataLength, data, readHeader->receptionReportOrSourceCount);
    return result;
}

RTCPHeader RTCPPackageHandler::readRTCPHeader(void* rtcpPackage, unsigned int packageLength)
{
    RTCPHeader *readHeader = (RTCPHeader *)rtcpPackage;
    RTCPHeader headerCopy = *readHeader;
    return headerCopy;
}

bool RTCPPackageHandler::isRTCPPackage(void* packageBuffer, unsigned int packageLength)
{
    //1. check for package size, if large enough
    if(packageLength < RTCP_HEADER_SIZE)
    {
        return false;
    }
    //2. we assume, it is an RTCP package
    RTCPHeader *readHeader = (RTCPHeader* )packageBuffer;

    //3. check for header-fields
    if(readHeader->version != 2)
    {
        //version is always 2 per specification
        return false;
    }
    switch(readHeader->packageType)
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
    if(getRTCPPackageLength(readHeader->length-1) > packageLength)
    {
        return false;
    }
    //we have no more fields to eliminate the package as RTCP-package, so we accept it
    return true;
}

const unsigned int RTCPPackageHandler::getRTCPPackageLength(unsigned int lengthHeaderField)
{
    //"The length of this RTCP packet in 32-bit words minus one, including the header and any padding"
    return (lengthHeaderField + 1) * 4;
}

const uint8_t RTCPPackageHandler::calculateLengthField(uint16_t length)
{
    //4 byte = 32 bit
    //we need to always round up, so we add 3:
    //e.g. 16 / 4 = 4 <-> (16+3) / 4 = 4
    //17 / 4 = 4 => (17+3) / 4 = 5!
    return ((length+3) / 4) - 1;
}
