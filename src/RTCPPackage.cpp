/* 
 * File:   RTCPPackage.cpp
 * Author: daniel
 * 
 * Created on June 16, 2015, 5:47 PM
 */

#include <malloc.h>
#include <string.h>

#include "RTCPPackage.h"

RTCPPackage::RTCPPackage()
{
    //maximal SR size: 8 (header) + 20 (sender-info) + 31 (5 bit RC count) * 128 (reception report)
    senderReportBuffer = new char[8 + 20 + 31 * 128];
    //maximal RR size: 8 (header) + 31 (5 bit RC count) * 128 (reception report)
    receiverReportBuffer = new char[8 + 21 * 128];
    //maximal BYE size: 8 (header) + 1 (length) + 255 (max 255 characters)
    byeBuffer = new char[8 + 1 + 255];
}

RTCPPackage::~RTCPPackage()
{
    free(senderReportBuffer);
    free(receiverReportBuffer);
    free(byeBuffer);
}


void* RTCPPackage::createSenderReportPackage(RTCPHeader& header, SenderInformation& senderInfo, std::vector<ReceptionReport> reports)
{
    header.packageType = RTCP_PACKAGE_SENDER_REPORT;
    header.receptionReportOrSourceCount = reports.size();
    memcpy(senderReportBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(senderReportBuffer + RTCP_HEADER_SIZE, &senderInfo, RTCP_SENDER_INFO_SIZE);
    //TODO reports
}

void* RTCPPackage::createReceiverReportPackage(RTCPHeader& header, std::vector<ReceptionReport> reports)
{

}

void* RTCPPackage::createByePackage(RTCPHeader& header, std::string byeMessage)
{
    header.packageType = RTCP_PACKAGE_GOODBYE;
    uint8_t length = byeMessage.size();
    memcpy(byeBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(byeBuffer + RTCP_HEADER_SIZE, &length, 1);
    memcpy(byeBuffer + RTCP_HEADER_SIZE + 1, byeMessage.c_str(), length);
    return byeBuffer;
}

std::vector<ReceptionReport> RTCPPackage::readSenderReport(void* senderReportPackage, uint16_t packageLength, RTCPHeader& header, SenderInformation& senderInfo)
{

}

std::vector<ReceptionReport> RTCPPackage::readReceiverReport(void* senderReportPackage, uint16_t packageLength, RTCPHeader& header)
{

}

std::string RTCPPackage::readByeMessage(void* senderReportPackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(byeBuffer, senderReportPackage, packageLength);
    RTCPHeader *readHeader = (RTCPHeader *)byeBuffer;
    header = *readHeader;
    uint8_t length = *(byeBuffer + RTCP_HEADER_SIZE);
    const char *byeMessage = (const char*)byeBuffer + RTCP_HEADER_SIZE + 1;
    return std::string(byeMessage, length);
}



