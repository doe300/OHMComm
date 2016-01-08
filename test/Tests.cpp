#include "Tests.h"

TestAudioIO::TestAudioIO() {
	TEST_ADD(TestAudioIO::testAudioHandlerInstances);
	TEST_ADD(TestAudioIO::testAudioProcessorInterface);
}

void TestAudioIO::testAudioHandlerInstances()
{
	auto audioHandler1 = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::RTAUDIO_WRAPPER);
	TEST_ASSERT_MSG(audioHandler1->isAudioConfigSet() == false, "AudioConfig has been set. 01");

	AudioConfiguration audioConfig = {0};
	audioHandler1->setConfiguration(audioConfig);
	TEST_ASSERT_MSG(audioHandler1->isAudioConfigSet() == true, "AudioConfig has not been set. 02");

	auto audioHandler2 = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::RTAUDIO_WRAPPER, audioConfig);
	TEST_ASSERT_MSG(audioHandler2->isAudioConfigSet() == true, "AudioConfig has not been set. 03");

	TEST_ASSERT_MSG(audioHandler2->getAudioConfiguration() == audioConfig, "AudioConfigs were not equal. 04");
	TEST_ASSERT_MSG(audioHandler1->getAudioConfiguration() == audioConfig, "AudioConfigs were not equal. 05");

	audioHandler2->reset();
	TEST_ASSERT_MSG(audioHandler2->isAudioConfigSet() == false, "AudioConfig has been set. 06");

	audioHandler2->setDefaultAudioConfig();
	TEST_ASSERT_MSG(audioHandler2->isAudioConfigSet() == true, "AudioConfig has not been set. 07");

	TEST_ASSERT_MSG((audioHandler2->getAudioConfiguration() == audioConfig) == false, "AudioConfigs were equal. 08");
}

void TestAudioIO::testAudioProcessorInterface()
{

}

int main(int argc, char** argv)
{
    #if TEST_OUTPUT_CONSOLE == 1
    Test::TextOutput output(Test::TextOutput::Verbose);
    #else
    std::ofstream file;
    file.open("testResult.log", std::ios_base::out | std::ios_base::trunc);

    Test::TextOutput output(Test::TextOutput::Verbose, file);
    #endif
    
    TestAudioIO testaudio;
    testaudio.run(output);

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
    
    TestSTUNClient testSTUN;
    testSTUN.run(output);
    
    TestUtility testUtility;
    testUtility.run(output);
}
