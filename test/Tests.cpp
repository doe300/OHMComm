#include "Tests.h"

int main(int argc, char** argv)
{
    #if TEST_OUTPUT_CONSOLE == 1
    Test::TextOutput output(Test::TextOutput::Verbose);
    #else
    std::ofstream file;
    file.open("testResult.log", std::ios_base::out | std::ios_base::trunc);

    Test::TextOutput output(Test::TextOutput::Verbose, file);
    #endif

    TestAudioHandler testAudio;
    testAudio.run(output);
    
    TestRTP testRTP;
    testRTP.run(output);

    TestUserInput testInput;
    testInput.run(output);

    TestParameters testParams;
    testParams.run(output);

    TestRTCP testRTCP;
    testRTCP.run(output);

    TestConfigurationModes testConfig;
    testConfig.run(output);

    TestNetworkWrappers testNetwork;
    testNetwork.run(output);

    TestRTPBuffer testBuffer;
    testBuffer.run(output);
    
    TestAudioProcessors testProcessors;
    testProcessors.run(output);
    
    TestSIPHandler testSIP;
    testSIP.run(output);
    
    TestSIPPackages testSIPPackages;
    testSIPPackages.run(output);
    
    TestSDP testSDP;
    testSDP.run(output);
    
    TestNetworkGrammars testNetGrammars;
    testNetGrammars.run(output);
    
    TestSocketAddress testSocketAddress;
    testSocketAddress.run(output);
    
    TestSIPGrammar testSIPGrammar;
    testSIPGrammar.run(output);
    
    TestSTUNClient testSTUN;
    testSTUN.run(output);
    
    TestUtility testUtility;
    testUtility.run(output);
    
    TestKeyValuePair testKeys;
    testKeys.run(output);
}
