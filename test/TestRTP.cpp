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
}

void TestRTP::testRTCPByeMessage()
{
    RTCPHeader *byeHeader = new RTCPHeader(RTCP_PACKAGE_APPLICATION_DEFINED, 0, 500);
    RTCPPackage p;
    
    void *byePointer = p.createByePackage(*byeHeader, std::string("Test massage"));
    byeHeader->packageType = RTCP_PACKAGE_RECEIVER_REPORT;
    std::string byeMessage = p.readByeMessage(byePointer, 21, *byeHeader);
    std::cout << byeMessage << ": " << (byeHeader->packageType+0) << std::endl;
}
