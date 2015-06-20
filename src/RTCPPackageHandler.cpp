/* 
 * File:   RTCPPackageHandler.cpp
 * Author: daniel
 * 
 * Created on June 16, 2015, 5:47 PM
 */

#include <malloc.h>
#include <string.h>

#include "RTCPPackageHandler.h"

RTCPPackageHandler::RTCPPackageHandler()
{
    //maximal SR size: 8 (header) + 20 (sender-info) + 31 (5 bit RC count) * 24 (reception report)
    senderReportBuffer = new char[8 + 20 + 31 * 24];
    //maximal RR size: 8 (header) + 31 (5 bit RC count) * 24 (reception report)
    receiverReportBuffer = new char[8 + 31 * 24];
    //maximal SDES size: 8 (header) + 31 (5 bit SDES count) * (1 (SDES type) + 1 (SDES length) + 255 (max 255 characters))
    sourceDescriptionBuffer = new char[8 + 31 * (1 + 1 + 255)];
    //maximal BYE size: 8 (header) + 1 (length) + 255 (max 255 characters)
    byeBuffer = new char[8 + 1 + 255];
}

RTCPPackageHandler::~RTCPPackageHandler()
{
    free(senderReportBuffer);
    free(receiverReportBuffer);
    free(sourceDescriptionBuffer);
    free(byeBuffer);
}


void* RTCPPackageHandler::createSenderReportPackage(RTCPHeader& header, SenderInformation& senderInfo, std::vector<ReceptionReport> reports)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_SENDER_REPORT;
    header.receptionReportOrSourceCount = reports.size();
    header.length = calculateLengthField(RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);
    
    memcpy(senderReportBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(senderReportBuffer + RTCP_HEADER_SIZE, &senderInfo, RTCP_SENDER_INFO_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(senderReportBuffer + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return senderReportBuffer;
}

void* RTCPPackageHandler::createReceiverReportPackage(RTCPHeader& header, std::vector<ReceptionReport> reports)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_RECEIVER_REPORT;
    header.receptionReportOrSourceCount = reports.size();
    header.length = calculateLengthField(RTCP_HEADER_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);
    
    memcpy(receiverReportBuffer, &header, RTCP_HEADER_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(senderReportBuffer + RTCP_HEADER_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return receiverReportBuffer;
}

void* RTCPPackageHandler::createSourceDescriptionPackage(RTCPHeader& header, std::vector<SourceDescription> descriptions)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_SOURCE_DESCRIPTION;
    header.receptionReportOrSourceCount = descriptions.size();

    uint16_t offset = RTCP_HEADER_SIZE;
    for(unsigned int i = 0; i < descriptions.size(); i++)
    {
        //set type
        memcpy(sourceDescriptionBuffer + offset, &descriptions[i].type, 1);
        offset++;
        //set length
        uint8_t size = descriptions[i].value.size();
        memcpy(sourceDescriptionBuffer + offset, &size, 1);
        offset++;
        //set value
        memcpy(sourceDescriptionBuffer + offset, descriptions[i].value.c_str(), size);
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
        memset(byeBuffer + offset, padding, padding);
    }
    
    //we need to copy the header last, because of the length- and padding-fields
    memcpy(sourceDescriptionBuffer, &header, RTCP_HEADER_SIZE);
    return sourceDescriptionBuffer;
}

void* RTCPPackageHandler::createByePackage(RTCPHeader& header, std::string byeMessage)
{
    header.packageType = RTCP_PACKAGE_GOODBYE;
    uint8_t length = byeMessage.size();
    //padding to the next multiple of 4 bytes
    uint8_t padding = 4 - ((1 + length) % 4);
    header.length = calculateLengthField(RTCP_HEADER_SIZE + 1 + length);
    header.padding = padding != 0;
    
    memcpy(byeBuffer, &header, RTCP_HEADER_SIZE);
    if(length > 0)
    {
        memcpy(byeBuffer + RTCP_HEADER_SIZE, &length, 1);
        memcpy(byeBuffer + RTCP_HEADER_SIZE + 1, byeMessage.c_str(), length);
    }
    if(padding != 0)
    {
        //apply padding - fill with number of padded bytes
        memset(byeBuffer + RTCP_HEADER_SIZE + 1 + length, padding, padding);
    }
    return byeBuffer;
}

std::vector<ReceptionReport> RTCPPackageHandler::readSenderReport(void* senderReportPackage, uint16_t packageLength, RTCPHeader& header, SenderInformation& senderInfo)
{
    memcpy(senderReportBuffer, senderReportPackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)senderReportBuffer;
    //copy header to out-parameter
    header = *readHeader;
    
    SenderInformation *readInfo = (SenderInformation *)senderReportBuffer + RTCP_HEADER_SIZE;
    //copy sender-info to out-parameter
    senderInfo = *readInfo;
    
    std::vector<ReceptionReport> reports(header.receptionReportOrSourceCount);
    for(unsigned int i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        ReceptionReport *readReport = (ReceptionReport *)senderReportBuffer + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + i * RTCP_RECEPTION_REPORT_SIZE;
        reports[i] = *readReport;
    }
    return reports;
}

std::vector<ReceptionReport> RTCPPackageHandler::readReceiverReport(void* receiverReportPackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(senderReportBuffer, receiverReportPackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)senderReportBuffer;
    //copy header to out-parameter
    header = *readHeader;
    
    std::vector<ReceptionReport> reports(header.receptionReportOrSourceCount);
    for(unsigned int i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        ReceptionReport *readReport = (ReceptionReport *)senderReportBuffer + RTCP_HEADER_SIZE + i * RTCP_RECEPTION_REPORT_SIZE;
        reports[i] = *readReport;
    }
    return reports;
}

std::vector<SourceDescription> RTCPPackageHandler::readSourceDescription(void* sourceDescriptionPackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(sourceDescriptionBuffer, sourceDescriptionPackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)byeBuffer;
    //copy header to out-parameter
    header = *readHeader;
    
    std::vector<SourceDescription> descriptions(header.receptionReportOrSourceCount);
    uint16_t offset = RTCP_HEADER_SIZE;
    for(uint8_t i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        RTCPSourceDescriptionType *type = (RTCPSourceDescriptionType*)senderReportBuffer + offset;
        offset++;
        uint8_t *valueLength = (uint8_t *)senderReportBuffer + offset;
        offset++;
        char *value = sourceDescriptionBuffer + offset;
        descriptions[i].type = *type;
        descriptions[i].value = std::string(value, *valueLength);
        offset += *valueLength;
    }
    return descriptions;
}


std::string RTCPPackageHandler::readByeMessage(void* byePackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(byeBuffer, byePackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)byeBuffer;
    //copy header to out-parameter
    header = *readHeader;
    uint8_t length = *(byeBuffer + RTCP_HEADER_SIZE);
    
    const char *byeMessage = (const char*)byeBuffer + RTCP_HEADER_SIZE + 1;
    return std::string(byeMessage, length);
}

uint8_t RTCPPackageHandler::calculateLengthField(uint16_t length)
{
    //4 byte = 32 bit
    //we need to always round up, so we add 3:
    //e.g. 16 / 4 = 4 <-> (16+3) / 4 = 4
    //17 / 4 = 4 => (17+3) / 4 = 5!
    return ((length+3) / 4) - 1;
}
