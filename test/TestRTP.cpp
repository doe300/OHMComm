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
    TEST_ADD(TestRTP::testRTPBuffer);
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
    
    //test empty message
    byePointer = p.createByePackage(*byeHeader, std::string());
    byeMessage = p.readByeMessage(byePointer, 21, *byeHeader);
    TEST_ASSERT_EQUALS_MSG(std::string(), byeMessage, "Empty message does not match! 04");
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

void TestRTP::testRTPBuffer()
{
    std::string payload("This is a dummy payload");
    uint16_t testTimestamp = 500;
    
    RTPBuffer buffer(5, 1000);
    RTPPackageHandler p(100);
    void *packageBuffer = p.getNewRTPPackage((void *)payload.c_str(), testTimestamp);
    void *receiveBuffer = p.getWorkBuffer();
    //we need the data in the receive-buffer, at least for the moment
    memcpy(receiveBuffer, packageBuffer, p.getSize());
    
    //write package to buffer
    TEST_ASSERT_EQUALS_MSG(RTP_BUFFER_ALL_OKAY, buffer.addPackage(p, payload.size()), "Error adding to buffer. 01");
    
    //assert size is 1
    TEST_ASSERT_EQUALS_MSG(1, buffer.getSize(), "Buffer-size does not match! 02");
    
    //overwrite receive-buffer with dummy-data
    memcpy(receiveBuffer, &testTimestamp, 50);
    
    //read package from buffer
    TEST_ASSERT_EQUALS_MSG(RTP_BUFFER_ALL_OKAY, buffer.readPackage(p), "Error reading from buffer. 03");
    
    //assert size is 0
    TEST_ASSERT_EQUALS_MSG(0, buffer.getSize(), "Buffer-size does not match! 04");
    
    //assert package header
    RTPHeader *readHeader = (RTPHeader *)p.getWorkBuffer();
    TEST_ASSERT_EQUALS_MSG(testTimestamp, readHeader->timestamp, "Timestamps do not match! 05");
    
    //some error assertions
    TEST_ASSERT_EQUALS_MSG(RTP_BUFFER_OUTPUT_UNDERFLOW, buffer.readPackage(p), "Output did not underflow! 06");
    
    //fill buffer
    for(int i = 0; i< 5; i++)
    {
        readHeader->sequence_number += 1;
        TEST_ASSERT_EQUALS_MSG(RTP_BUFFER_ALL_OKAY, buffer.addPackage(p, payload.size()), "Error adding to buffer. 07");
    }
    //TODO doesn't work yet!
    readHeader->sequence_number += 1;
    TEST_ASSERT_EQUALS_MSG(RTP_BUFFER_INPUT_OVERFLOW, buffer.addPackage(p, payload.size()), "Input did not overflow. 08");
}
