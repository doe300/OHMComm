/* 
 * File:   TestRTP.cpp
 * Author: daniel
 * 
 * Created on June 16, 2015, 8:26 PM
 */

#include "TestRTP.h"

TestRTP::TestRTP()
{
    TEST_ADD(TestRTP::testRTCPByeMessage);
    TEST_ADD(TestRTP::testRTPPackage);
}

void TestRTP::testRTCPByeMessage()
{
    std::string testMessage("Test massage");
    
    RTCPHeader *byeHeader = new RTCPHeader(RTCP_PACKAGE_APPLICATION_DEFINED, 0, 500);
    RTCPPackageHandler p;
    void *byePointer = p.createByePackage(*byeHeader, testMessage);
    //package-type should be adjusted
    TEST_ASSERT_EQUALS_MSG(RTCP_PACKAGE_GOODBYE, byeHeader->packageType, "Package-type of GOODBYE expected! 01");
    
    //modify package-type again
    byeHeader->packageType = RTCP_PACKAGE_RECEIVER_REPORT;
    
    std::string byeMessage = p.readByeMessage(byePointer, 21, *byeHeader);
    //package-type should be adjusted again
    TEST_ASSERT_EQUALS_MSG(RTCP_PACKAGE_GOODBYE, byeHeader->packageType, "Package-type of GOODBYE expected! 02");
    TEST_ASSERT_EQUALS_MSG(testMessage, byeMessage, "Messages don't match! 03")
}

void TestRTP::testRTPPackage()
{
    std::string payload("This is a dummy payload");
    uint16_t testTimestamp = 500;
	RTPPackageHandler pack(100, PayloadType::GSM, RTP_HEADER_MIN_SIZE);
    
    void *packageBuffer = pack.getNewRTPPackage((char *)payload.c_str(), testTimestamp);
    
    void *headerBuffer = pack.getRTPPackageHeader(packageBuffer);
    RTPHeader *header = (RTPHeader *)headerBuffer;
    TEST_ASSERT_EQUALS_MSG(header->payload_type, PayloadType::GSM, "Payload types don't match! 01");
    TEST_ASSERT_EQUALS_MSG(header->timestamp, testTimestamp, "Timestamps don't match! 02");
    
    void *contentBuffer = pack.getRTPPackageData(packageBuffer);
    TEST_ASSERT_EQUALS_MSG(memcmp(payload.c_str(), contentBuffer, payload.size()), 0, "Payloads don't match! 03");
}
