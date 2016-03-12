/*
 * File:   TestParameters.cpp
 * Author: daniel
 *
 * Created on July 24, 2015, 1:41 PM
 */

#include "TestParameters.h"

using namespace ohmcomm;

TestParameters::TestParameters() : params({},{})
{
    TEST_ADD(TestParameters::testParseParameters);
    TEST_ADD(TestParameters::testIsParameterSet);
    TEST_ADD(TestParameters::testGetParameterValue);
    TEST_ADD(TestParameters::testGetAudioProcessors);
}

void TestParameters::testParseParameters()
{
    TEST_ASSERT_MSG(false == params.parseParameters(0, nullptr), "Empty parameters not detected");
    char* arguments[] = {
        (char*)"progName",
        (char*)"--remote-address=127.0.0.1",
        (char*)"-a=Proc1",
        (char*)"-a=Proc2"
    };
    TEST_ASSERT_MSG(params.parseParameters(4, arguments) == true, "Parameter parsing failed");
}

void TestParameters::testIsParameterSet()
{
    TEST_ASSERT_MSG(params.isParameterSet(Parameters::REMOTE_ADDRESS), "Parameter set not detected");
    TEST_ASSERT_MSG(params.isParameterSet(Parameters::HELP) == false, "Parameter wrongly found");
}

void TestParameters::testGetParameterValue()
{
    TEST_ASSERT_EQUALS_MSG("127.0.0.1", params.getParameterValue(Parameters::REMOTE_ADDRESS), "Parameter value wrong");
    //test for default value
    TEST_ASSERT_EQUALS_MSG(std::to_string(DEFAULT_NETWORK_PORT), params.getParameterValue(Parameters::REMOTE_PORT), "Default value not returned");
}

void TestParameters::testGetAudioProcessors()
{
    TEST_ASSERT_EQUALS_MSG(2, params.getAudioProcessors().size(), "AudioProcessors not detected correctly");
    TEST_ASSERT_EQUALS_MSG("Proc1", params.getAudioProcessors()[0], "AudioProcessor-names wrong");
    TEST_ASSERT_EQUALS_MSG("Proc2", params.getAudioProcessors()[1], "AudioProcessor-names wrong");
}
