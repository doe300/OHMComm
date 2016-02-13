/* 
 * File:   TestSIPHandler.cpp
 * Author: daniel
 * 
 * Created on November 26, 2015, 1:04 PM
 */

#include "TestSIPHandler.h"
#include "network/UDPWrapper.h"

TestSIPHandler::TestSIPHandler() : Suite(), handler(nullptr)
{
//    TEST_ADD(TestSIPHandler::testSIPThread);
#ifdef EXTENDED_SIP_TEST
    //we successfully handle tests 1 to 190
    for(unsigned short i = 191; i < 4527; ++i)
    {
        TEST_ADD_WITH_INTEGER(TestSIPHandler::testSIPProtocol, i);
    }
#endif
}

void TestSIPHandler::testSIPThread()
{
    handler->startUp();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    handler->shutdown();
}

void TestSIPHandler::testSIPProtocol(const int index)
{
    //Test cases and jar from: https://www.ee.oulu.fi/research/ouspg/PROTOS_Test-Suite_c07-sip
    //Fails: 
    /*
     * - SIP-Request-URI (doesn't verify existence of remote before accepting??)
     * - SIP-Via-Host ()
     */
    //make sure, handler is running before
    TEST_ASSERT(handler->isRunning());
    //For this test to work, the test-suite from https://www.ee.oulu.fi/research/ouspg/PROTOS_Test-Suite_c07-sip must be available locally
    const std::string testCommand = std::string("java -jar c07-sip-r2.jar -touri test@localhost -dport 2060 -lport 5060 -single ") + std::to_string(index);
    system(testCommand.data());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //test if handler is still running
    TEST_ASSERT(handler->isRunning());
}

bool TestSIPHandler::before(const std::string& methodName)
{
    handler.reset(new SIPHandler({2060, "192.168.178.123", SIPHandler::SIP_DEFAULT_PORT}, "dummy", 
        [this](const MediaDescription, const NetworkConfiguration, const NetworkConfiguration){dummyHandler();}));
    handler->startUp();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
}

void TestSIPHandler::after(const std::string& methodName, const bool success)
{
    handler->shutdown();
}

void TestSIPHandler::dummyHandler()
{
    TEST_FAIL("Wrong configuration accepted!");
}
