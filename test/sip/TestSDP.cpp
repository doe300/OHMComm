/* 
 * File:   TestSDP.cpp
 * Author: doe300
 * 
 * Created on April 18, 2016, 5:19 PM
 */

#include "TestSDP.h"

using namespace ohmcomm::sip;

TestSDP::TestSDP()
{
    TEST_ADD(TestSDP::testSessionDescription);
    TEST_ADD(TestSDP::testMediaDescription);
}

void TestSDP::testSessionDescription()
{
    const std::string descrString = SDPMessageHandler::createSessionDescription("user", ohmcomm::NetworkConfiguration{12345, "127.0.0.1", 12345});
    const SessionDescription descr = SDPMessageHandler::readSessionDescription(descrString);
    SDPMessageHandler::checkSessionDescription(&descr);
    
    TEST_ASSERT_EQUALS(0, descr.getConnectionAddress().compare("127.0.0.1"));
    TEST_ASSERT(!descr.getAttribute("tool").empty());
    TEST_ASSERT(descr.hasKey(SessionDescription::SDP_MEDIA));
}

void TestSDP::testMediaDescription()
{
    const std::string descrString = SDPMessageHandler::createSessionDescription("user", ohmcomm::NetworkConfiguration{12345, "127.0.0.1", 12345});
    const SessionDescription descr = SDPMessageHandler::readSessionDescription(descrString);
    SDPMessageHandler::checkSessionDescription(&descr);
    const std::vector<MediaDescription> media = SDPMessageHandler::readMediaDescriptions(descr);
    
    TEST_ASSERT_EQUALS(SupportedFormats::getFormats().size(), media.size());
    for(const MediaDescription& m : media)
    {
        TEST_ASSERT_EQUALS(0, m.protocol.compare(SessionDescription::SDP_MEDIA_RTP));
        TEST_ASSERT(m.getFormat().payloadType >= 0);
    }
}
