#include "Tests.h"

TestAudioIO::TestAudioIO() {
	TEST_ADD(TestAudioIO::testAudioHandlerInstances);
	TEST_ADD(TestAudioIO::testAudioProcessorInterface);
}

void TestAudioIO::testAudioHandlerInstances()
{
	auto audioHandler1 = AudioHandlerFactory::getAudioHandler("rtaudioWRaPPeR");
	TEST_ASSERT_MSG(audioHandler1->IsAudioConfigSet() == false, "AudioConfig has been set. 01");
	
	AudioConfiguration audioConfig = {0};
	audioHandler1->setConfiguration(audioConfig);
	TEST_ASSERT_MSG(audioHandler1->IsAudioConfigSet() == true, "AudioConfig has not been set. 02");

	auto audioHandler2 = AudioHandlerFactory::getAudioHandler("rtaudiowrapper", audioConfig);
	TEST_ASSERT_MSG(audioHandler2->IsAudioConfigSet() == true, "AudioConfig has not been set. 03");

	TEST_ASSERT_MSG(audioHandler2->getAudioConfiguration() == audioConfig, "AudioConfigs were not equal. 04");
	TEST_ASSERT_MSG(audioHandler1->getAudioConfiguration() == audioConfig, "AudioConfigs were not equal. 05");

	audioHandler2->reset();
	TEST_ASSERT_MSG(audioHandler2->IsAudioConfigSet() == false, "AudioConfig has been set. 06");

	audioHandler2->setDefaultAudioConfig();
	TEST_ASSERT_MSG(audioHandler2->IsAudioConfigSet() == true, "AudioConfig has not been set. 07");

	TEST_ASSERT_MSG((audioHandler2->getAudioConfiguration() == audioConfig) == false, "AudioConfigs were equal. 08");
}

void TestAudioIO::testAudioProcessorInterface()
{
    
}

int main(int argc, char** argv)
{
    Test::TextOutput output(Test::TextOutput::Verbose);
    TestAudioIO testaudio;
    testaudio.run(output);
    
    TestRTP testRTP;
    testRTP.run(output);
}