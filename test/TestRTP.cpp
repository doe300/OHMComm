/*
 * File:   TestRTP.cpp
 * Author: daniel
 *
 * Created on June 16, 2015, 8:26 PM
 */

#include "TestRTP.h"

TestRTP::TestRTP() : Test::Suite()
{
    TEST_ADD(TestRTP::testRTPPackage);
}

void TestRTP::testRTPPackage()
{
    std::string payload("This is a dummy payload");
    RTPPackageHandler pack(100, PayloadType::GSM, RTP_HEADER_MIN_SIZE);

    void *packageBuffer = pack.getNewRTPPackage((char *)payload.c_str(), payload.size());

    void *headerBuffer = pack.getRTPPackageHeader(packageBuffer);
    RTPHeader *header = (RTPHeader *)headerBuffer;
    TEST_ASSERT_EQUALS_MSG(header->payload_type, PayloadType::GSM, "Payload types don't match! 01");

    void *contentBuffer = pack.getRTPPackageData(packageBuffer);
    TEST_ASSERT_EQUALS_MSG(memcmp(payload.c_str(), contentBuffer, payload.size()), 0, "Payloads don't match! 02");
    
    pack.setActualPayloadSize(payload.length());
    TEST_ASSERT(pack.getMaximumPackageSize() >= pack.getRTPHeaderSize() + pack.getMaximumPayloadSize());
    TEST_ASSERT(pack.getActualPayloadSize() <= pack.getMaximumPayloadSize());
    TEST_ASSERT(RTPPackageHandler::isRTPPackage(pack.getWorkBuffer(), pack.getActualPayloadSize()));
}
