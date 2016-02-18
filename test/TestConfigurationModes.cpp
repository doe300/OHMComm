#include "TestConfigurationModes.h"
#include "AudioProcessorFactory.h"
#include "AudioHandlerFactory.h"
#include "OHMComm.h"

#include <string>
#include <vector>

TestConfigurationModes::TestConfigurationModes()
{
    TEST_ADD(TestConfigurationModes::testParameterConfiguration);
    TEST_ADD(TestConfigurationModes::testInteractiveConfiguration);
    TEST_ADD(TestConfigurationModes::testLibraryConfiguration);
    TEST_ADD(TestConfigurationModes::testPassiveConfiguration);
    TEST_ADD(TestConfigurationModes::testFileConfiguration);
    TEST_ADD(TestConfigurationModes::testSIPConfiguration);
}

void TestConfigurationModes::testParameterConfiguration()
{
    char* args[] = {(char*)"Tests", (char*)"-r=127.0.0.1", (char*)"-l=33333", (char*)"-p=33333", (char*)"-a=Opus-Codec", (char*)"--log-file=test.log", (char*)"-S=48000", (char*)"-t"};
    Parameters params({},{});
    TEST_ASSERT_MSG(params.parseParameters(8, args), "Failed to parse parameters!");

    ConfigurationMode* mode = new ParameterConfiguration(params);

    TEST_ASSERT_MSG(mode->isConfigured(), "Parameter-configuration not finished!");
    TEST_ASSERT_MSG(mode->runConfiguration(), "Parameter-configuration not finished!");

    TEST_ASSERT_EQUALS("127.0.0.1", mode->getNetworkConfiguration().remoteIPAddress);
    TEST_ASSERT_EQUALS(33333, mode->getNetworkConfiguration().remotePort);

    TEST_ASSERT_MSG(mode->getLogToFileConfiguration().first, "Log to file not configured!");
    TEST_ASSERT_EQUALS(48000, mode->getAudioConfiguration().forceSampleRate);

    std::vector<std::string> tmp(0);
    TEST_ASSERT_MSG(mode->getAudioProcessorsConfiguration(tmp), "Profile processors not configured!");
    TEST_ASSERT_EQUALS(AudioProcessorFactory::OPUS_CODEC, tmp[0]);
    
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
    //don't wait for passive configuration
    testStream << 'n' << std::endl;
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

    TEST_ASSERT_MSG(false == mode->getAudioHandlerConfiguration().second, "default audio-config was not set!");
    TEST_ASSERT_EQUALS(DEFAULT_NETWORK_PORT, mode->getNetworkConfiguration().localPort);
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
    ((LibraryConfiguration*)mode)->configureLogToFile("test.log");
    ((LibraryConfiguration*)mode)->configureProcessors({AudioProcessorFactory::OPUS_CODEC}, true);
    std::vector<std::string> tmp;
    TEST_ASSERT_MSG(mode->getAudioProcessorsConfiguration(tmp), "Processor-profiling not set!");
    TEST_ASSERT_EQUALS(AudioProcessorFactory::OPUS_CODEC, tmp[0]);
    
    TEST_ASSERT_MSG(!mode->getAudioHandlerConfiguration().second, "default audio-configuration was not used!");
    TEST_ASSERT_EQUALS("test.log", mode->getLogToFileConfiguration().second);
    TEST_ASSERT_EQUALS(DEFAULT_NETWORK_PORT, mode->getNetworkConfiguration().localPort);
    
    ((LibraryConfiguration*)mode)->configureCustomValue("someKey", 123);
    TEST_ASSERT_EQUALS(123, mode->getCustomConfiguration("someKey", "", 42));

    delete mode;
}

void TestConfigurationModes::testPassiveConfiguration()
{
    //TODO rewrite to create new thread!
    TEST_FAIL("Currently disabled on purpose!");
    return;
    //set up "remote"
    LibraryConfiguration* remoteMode = new LibraryConfiguration();
    NetworkConfiguration remoteNetwork;
    remoteNetwork.remoteIPAddress = "127.0.0.1";
    remoteNetwork.remotePort = DEFAULT_NETWORK_PORT+1;
    remoteMode->configureAudio(AudioHandlerFactory::RTAUDIO_WRAPPER, nullptr);
    remoteMode->configureNetwork(remoteNetwork);
    std::vector<std::string> remoteProcessorNames = {AudioProcessorFactory::OPUS_CODEC};
    remoteMode->configureProcessors(remoteProcessorNames, false);
    OHMComm remote((ConfigurationMode*)remoteMode);
    TEST_ASSERT_MSG(remote.isConfigurationDone(false), "Remote not configured!");
    //FIXME won't work, thread stalls until request is done
    remote.startAudioThreads();
    
    //set up "client"
    NetworkConfiguration netConf;
    netConf.remoteIPAddress = "127.0.0.1";
    netConf.localPort = DEFAULT_NETWORK_PORT+1;

    ConfigurationMode* mode = new PassiveConfiguration(netConf);

    TEST_ASSERT_EQUALS(netConf.remoteIPAddress, mode->getNetworkConfiguration().remoteIPAddress);

    TEST_ASSERT_MSG(mode->runConfiguration(), "Failed to configure passively!");

    TEST_ASSERT_MSG(mode->getAudioConfiguration().forceAudioFormatFlag != 0, "Forcing audio-format was not set!");
    TEST_ASSERT_MSG(mode->getAudioConfiguration().forceSampleRate != 0, "Forcing sample-rate was not set!");
    std::vector<std::string> passiveProcessorNames;
    mode->getAudioProcessorsConfiguration(passiveProcessorNames);
    TEST_ASSERT_EQUALS(1, passiveProcessorNames.size());
    TEST_ASSERT_EQUALS(AudioProcessorFactory::OPUS_CODEC, passiveProcessorNames[0]);

    remote.stopAudioThreads();
    delete remoteMode;
    delete mode;
}

void TestConfigurationModes::testFileConfiguration()
{
    char* args[] = {(char*)"progName", (char*)"../test/test.config"};
    Parameters params({},{});
    params.parseParameters(2, args);
    ConfigurationMode* mode = new ParameterConfiguration(params);
    
    TEST_ASSERT_MSG(mode->runConfiguration(), "Reading configuration-file failed!");
    TEST_ASSERT_EQUALS(true, mode->getLogToFileConfiguration().first);
    TEST_ASSERT_EQUALS(std::string("somelog.log"), mode->getLogToFileConfiguration().second);
    std::vector<std::string> procNames;
    TEST_ASSERT_MSG(mode->getAudioProcessorsConfiguration(procNames), "Profiling-processors not configured");
    TEST_ASSERT_EQUALS(AudioProcessorFactory::OPUS_CODEC, procNames[0]);
    TEST_ASSERT_EQUALS(AudioProcessorFactory::WAV_WRITER, procNames[1]);
    const NetworkConfiguration netConf = mode->getNetworkConfiguration();
    TEST_ASSERT_EQUALS(std::string("127.0.0.1"), netConf.remoteIPAddress);
    TEST_ASSERT_EQUALS(54321, netConf.localPort);
    const AudioConfiguration audioConf = mode->getAudioConfiguration();
    TEST_ASSERT_EQUALS(44100, audioConf.sampleRate);
    TEST_ASSERT_EQUALS(2, audioConf.audioFormatFlag);
    
    delete mode;
}

void TestConfigurationModes::testSIPConfiguration()
{
    const Parameters params({}, {});
    ConfigurationMode* config = new SIPConfiguration(params, {54321, "127.0.0.1", SIPHandler::SIP_DEFAULT_PORT});
    TEST_ASSERT_EQUALS(false, config->isConfigured());
    TEST_ASSERT(config->runConfiguration());
    TEST_ASSERT(config->isConfigured());
    
    //test specific values
    TEST_ASSERT_EQUALS(48000, config->getAudioConfiguration().forceSampleRate);
    std::vector<std::string> procNames;
    TEST_ASSERT(!config->getAudioProcessorsConfiguration(procNames));
    TEST_ASSERT(procNames.size() == 1);
    TEST_ASSERT(procNames[0].compare(AudioProcessorFactory::OPUS_CODEC) == 0);
}
