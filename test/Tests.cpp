#include "Tests.h"

static const unsigned char TEST_AUDIO = 0x01;
static const unsigned char TEST_RTP = 0x02;
static const unsigned char TEST_CONFIG = 0x04;
static const unsigned char TEST_NETWORK = 0x08;
static const unsigned char TEST_SIP = 0x10;
static const unsigned char TEST_UTIL = 0x20;

int main(int argc, char** argv)
{
    #if TEST_OUTPUT_CONSOLE == 1
    Test::TextOutput output(Test::TextOutput::Verbose);
    #else
    std::ofstream file;
    file.open("testResult.log", std::ios_base::out | std::ios_base::trunc);

    Test::TextOutput output(Test::TextOutput::Verbose, file);
    #endif

    unsigned char runTests = 0;
    //to select custom tests
    if(argc == 1)
    {
        runTests = 0xFF;
    }
    else
    {
        for(int i = 1; i < argc; ++i)
        {
            if(strcmp("audio", argv[i]) == 0)
            {
                runTests |= TEST_AUDIO;
            }
            else if(strcmp("rtp", argv[1]) == 0)
            {
                runTests |= TEST_RTP;
            }
            else if(strcmp("config", argv[1]) == 0)
            {
                runTests |= TEST_CONFIG;
            }
            else if(strcmp("network", argv[1]) == 0)
            {
                runTests |= TEST_NETWORK;
            }
            else if(strcmp("sip", argv[1]) == 0)
            {
                runTests |= TEST_SIP;
            }
            else if(strcmp("util", argv[1]) == 0)
            {
                runTests |= TEST_UTIL;
            }
        }
    }

    if(runTests & TEST_AUDIO)
    {
        TestAudioHandler testAudio;
        testAudio.run(output);
        
        TestAudioProcessors testProcessors;
        testProcessors.run(output);
    }
    if(runTests & TEST_RTP)
    {
        TestRTP testRTP;
        testRTP.run(output);
        
        TestRTCP testRTCP;
        testRTCP.run(output);
        
        TestRTPBuffer testBuffer;
        testBuffer.run(output);
    }
    if(runTests & TEST_CONFIG)
    {
        TestUserInput testInput;
        testInput.run(output);

        TestParameters testParams;
        testParams.run(output);
        
        TestConfigurationModes testConfig;
        testConfig.run(output);
    }
    if(runTests & TEST_NETWORK)
    {
        TestNetworkWrappers testNetwork;
        testNetwork.run(output);
        
        TestNetworkGrammars testNetGrammars;
        testNetGrammars.run(output);

        TestSocketAddress testSocketAddress;
        testSocketAddress.run(output);
        
        TestSTUNClient testSTUN;
        testSTUN.run(output);
    }
    if(runTests & TEST_SIP)
    {
        TestSIPHandler testSIP;
        testSIP.run(output);

        TestSIPPackages testSIPPackages;
        testSIPPackages.run(output);

        TestSDP testSDP;
        testSDP.run(output);
        
        TestSIPGrammar testSIPGrammar;
        testSIPGrammar.run(output);

        TestSIPRequest testSIPRequests;
        testSIPRequests.run(output);
    }
    if(runTests & TEST_UTIL)
    {
        TestUtility testUtility;
        testUtility.run(output);

        TestKeyValuePair testKeys;
        testKeys.run(output);
    }
}
