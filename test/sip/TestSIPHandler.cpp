/* 
 * File:   TestSIPHandler.cpp
 * Author: daniel
 * 
 * Created on November 26, 2015, 1:04 PM
 */

#include "TestSIPHandler.h"
#include "UDPWrapper.h"

TestSIPHandler::TestSIPHandler() : Suite(), handler({54321, "127.0.0.1", SIP_DEFAULT_PORT}, "doe300", "localhost", "doe301", SIPHandler::generateCallID("test-host"))
{
    TEST_ADD(TestSIPHandler::testSIPThread);
}

void TestSIPHandler::testSIPThread()
{
    handler.startUp();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    handler.shutdown();
}

