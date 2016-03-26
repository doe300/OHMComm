/*
 * File:   TestRTP.cpp
 * Author: daniel
 *
 * Created on June 16, 2015, 8:26 PM
 */

#include "TestRTP.h"

using namespace ohmcomm::rtp;

TestRTP::TestRTP() : Test::Suite()
{
    TEST_ADD(TestRTP::testRTPPackage);
}

void TestRTP::testRTPPackage()
{
    Participant part(15,true);
    part.payloadType = ohmcomm::PayloadType::GSM;
    std::string payload("This is a dummy payload");
    RTPPackageHandler pack(100, part);

    pack.createNewRTPPackage((char *)payload.c_str(), payload.size());
    const void* headerBuffer = pack.getRTPPackageHeader();
    RTPHeader* header = (RTPHeader *)headerBuffer;
    TEST_ASSERT_EQUALS_MSG(header->getPayloadType(), ohmcomm::PayloadType::GSM, "Payload types don't match! 01");

    const void* contentBuffer = pack.getRTPPackageData();
    TEST_ASSERT_EQUALS_MSG(memcmp(payload.c_str(), contentBuffer, payload.size()), 0, "Payloads don't match! 02");
    
    pack.setActualPayloadSize(payload.length());
    TEST_ASSERT(pack.getMaximumPackageSize() >= pack.getRTPHeaderSize() + pack.getMaximumPayloadSize());
    TEST_ASSERT(pack.getActualPayloadSize() <= pack.getMaximumPayloadSize());
    TEST_ASSERT(RTPPackageHandler::isRTPPackage(pack.getWorkBuffer(), pack.getActualPayloadSize()));
}
