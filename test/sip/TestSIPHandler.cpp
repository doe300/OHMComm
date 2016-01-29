/* 
 * File:   TestSIPHandler.cpp
 * Author: daniel
 * 
 * Created on November 26, 2015, 1:04 PM
 */

#include "TestSIPHandler.h"
#include "UDPWrapper.h"

TestSIPHandler::TestSIPHandler() : Suite(), handler({2060, "192.168.178.123", SIP_DEFAULT_PORT}, "dummy", [this](const MediaDescription, const NetworkConfiguration, const NetworkConfiguration){TEST_FAIL("Wrong configuration accepted!");})
{
//    TEST_ADD(TestSIPHandler::testSIPThread);
#ifdef EXTENDED_SIP_TEST
    for(unsigned short i = 1; i < 4527; ++i)
    {
        TEST_ADD_WITH_INTEGER(TestSIPHandler::testSIPProtocol, i);
    }
#endif
}

void TestSIPHandler::testSIPThread()
{
    handler.startUp();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    handler.shutdown();
}

void TestSIPHandler::testSIPProtocol(const int index)
{
    //make sure, handler is running before
    TEST_ASSERT(handler.isRunning() || setup());
    //For this test to work, the test-suite from https://www.ee.oulu.fi/research/ouspg/PROTOS_Test-Suite_c07-sip must be available locally
    const std::string testCommand = std::string("java -jar c07-sip-r2.jar -touri test@localhost -dport 2060 -lport 5060 -single ") + std::to_string(index);
    system(testCommand.data());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //test if handler is still running
    TEST_ASSERT(handler.isRunning());
}

bool TestSIPHandler::setup()
{
    handler.startUp();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return true;
}

void TestSIPHandler::tear_down()
{
    handler.shutdown();
}

