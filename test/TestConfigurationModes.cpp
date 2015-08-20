#include "TestConfigurationModes.h"
#include "AudioProcessorFactory.h"

#include <string>
#include <vector>

TestConfigurationModes::TestConfigurationModes()
{
    TEST_ADD(TestConfigurationModes::testParameterConfiguration);
    TEST_ADD(TestConfigurationModes::testInteractiveConfiguration);
    TEST_ADD(TestConfigurationModes::testLibraryConfiguration);
    TEST_ADD(TestConfigurationModes::testPassiveConfiguration);
}

void TestConfigurationModes::testParameterConfiguration()
{
    char* args[] = {(char*)"Tests", (char*)"-r=127.0.0.1", (char*)"-l=33333", (char*)"-p=33333", (char*)"-a=Opus-Codec", (char*)"--log-file=test.log"};
    Parameters params;
    TEST_ASSERT_MSG(params.parseParameters(5, args, AudioProcessorFactory::getAudioProcessorNames()), "Failed to parse parameters!");

    ConfigurationMode* mode = new ParameterConfiguration(params);

    TEST_ASSERT_MSG(mode->isConfigured(), "Parameter-configuration not finished!");
    TEST_ASSERT_MSG(mode->runConfiguration(), "Parameter-configuration not finished!");

    TEST_ASSERT_EQUALS("127.0.0.1", mode->getNetworkConfiguration().addressOutgoing);
    TEST_ASSERT_EQUALS(33333, mode->getNetworkConfiguration().portOutgoing);

    TEST_ASSERT_MSG(mode->getLogToFileConfiguration().first, "Log to file not configured!");

    delete mode;
}

void TestConfigurationModes::testInteractiveConfiguration()
{
    ConfigurationMode* mode = new InteractiveConfiguration();

    TEST_ASSERT_MSG(!mode->isConfigured(), "interactive configuration wrongly finished!");

    //need to emulate input, see TestUserInput
    std::streambuf* origStdinBuf;
    std::stringstream testStream;
    //save original input buffer
    origStdinBuf = std::cin.rdbuf();
    testStream.clear();
    std::cin.rdbuf(testStream.rdbuf());

    //emulate input
    //default audio-config
    testStream << 'y' << std::endl;
    //default network config
    testStream << 'y' << std::endl;
    //profile processors
    testStream << 'y' << std::endl;
    //log statistics
    testStream << 'y' << std::endl;
    //log file
    testStream << "test.log" << std::endl;
    //processors (add Opus and close)
    testStream << 0 << std::endl << AudioProcessorFactory::getAudioProcessorNames().size() << std::endl;


    TEST_ASSERT_MSG(mode->runConfiguration(), "interactive configuration was not finished!");
    //restore original standard-input
    std::cin.rdbuf(origStdinBuf);

    TEST_ASSERT(mode->isConfigured());

    TEST_ASSERT_MSG(mode->getAudioHandlerConfiguration().second, "default audio-config was not set!");
    TEST_ASSERT_EQUALS(DEFAULT_NETWORK_PORT, mode->getNetworkConfiguration().portIncoming);
    TEST_ASSERT_MSG(mode->getLogToFileConfiguration().first, "Log to file was not configured!");
    TEST_ASSERT_EQUALS("test.log", mode->getLogToFileConfiguration().second);
    std::vector<std::string> tmp;
    TEST_ASSERT_MSG(mode->getAudioProcessorsConfiguration(tmp), "Profile processors not configured!");
    TEST_ASSERT_EQUALS(AudioProcessorFactory::getAudioProcessorNames()[0], tmp[0]);

    delete mode;
}

void TestConfigurationModes::testLibraryConfiguration()
{
    ConfigurationMode* mode = new LibraryConfiguration();

    TEST_ASSERT_MSG(!mode->isConfigured(), "Library-configuration wrongly finished!");

    ((LibraryConfiguration*)mode)->configureAudio(AudioHandlerFactory::RTAUDIO_WRAPPER, nullptr);
    TEST_ASSERT_MSG(!mode->getAudioHandlerConfiguration().second, "default audio-configuration was not used!");

    ((LibraryConfiguration*)mode)->configureLogToFile("test.log");
    TEST_ASSERT_EQUALS("test.log", mode->getLogToFileConfiguration().second);

    TEST_ASSERT_EQUALS(DEFAULT_NETWORK_PORT, mode->getNetworkConfiguration().portIncoming);

    ((LibraryConfiguration*)mode)->configureProcessors({AudioProcessorFactory::OPUS_CODEC}, true);
    std::vector<std::string> tmp;
    TEST_ASSERT_MSG(mode->getAudioProcessorsConfiguration(tmp), "Processor-profiling not set!");
    TEST_ASSERT_EQUALS(AudioProcessorFactory::OPUS_CODEC, tmp[0]);

    delete mode;
}

void TestConfigurationModes::testPassiveConfiguration()
{
    NetworkConfiguration netConf;
    netConf.addressOutgoing = "127.0.0.1";

    ConfigurationMode* mode = new PassiveConfiguration(netConf);

    TEST_ASSERT_EQUALS(netConf.addressOutgoing, mode->getNetworkConfiguration().addressOutgoing);

    //TODO implementation of other side
    TEST_ASSERT_MSG(mode->runConfiguration(), "Failed to configure passively!");

    delete mode;
}
