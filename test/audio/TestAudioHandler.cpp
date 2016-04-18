/* 
 * File:   TestAudioHandler.cpp
 * Author: doe300
 * 
 * Created on April 18, 2016, 4:55 PM
 */

#include "TestAudioHandler.h"
#include "processors/AudioProcessorFactory.h"

using namespace ohmcomm;

TestAudioHandler::TestAudioHandler()
{
    TEST_ADD_WITH_STRING(TestAudioHandler::testAudioHandlerInstances, AudioHandlerFactory::RTAUDIO_WRAPPER);
#ifdef PORTAUDIO_HEADER
    TEST_ADD_WITH_STRING(TestAudioHandler::testAudioHandlerInstances, AudioHandlerFactory::PORTAUDIO_WRAPPER);
#endif
    TEST_ADD(TestAudioHandler::testAudioProcessorInterface);
    TEST_ADD_WITH_STRING(TestAudioHandler::testAudioDevices, AudioHandlerFactory::RTAUDIO_WRAPPER);
#ifdef PORTAUDIO_HEADER
    TEST_ADD_WITH_STRING(TestAudioHandler::testAudioDevices, AudioHandlerFactory::PORTAUDIO_WRAPPER);
#endif
}

void TestAudioHandler::testAudioHandlerInstances(const std::string processorName)
{
    auto audioHandler1 = AudioHandlerFactory::getAudioHandler(processorName);
    TEST_ASSERT_MSG(audioHandler1->isAudioConfigSet() == false, "AudioConfig has been set. 01");

    AudioConfiguration audioConfig = {0};
    audioHandler1->setConfiguration(audioConfig);
    TEST_ASSERT_MSG(audioHandler1->isAudioConfigSet() == true, "AudioConfig has not been set. 02");

    auto audioHandler2 = AudioHandlerFactory::getAudioHandler(processorName, audioConfig);
    TEST_ASSERT_MSG(audioHandler2->isAudioConfigSet() == true, "AudioConfig has not been set. 03");

    TEST_ASSERT_MSG(audioHandler2->getAudioConfiguration() == audioConfig, "AudioConfigs were not equal. 04");
    TEST_ASSERT_MSG(audioHandler1->getAudioConfiguration() == audioConfig, "AudioConfigs were not equal. 05");

    audioHandler2->reset();
    TEST_ASSERT_MSG(audioHandler2->isAudioConfigSet() == false, "AudioConfig has been set. 06");

    audioHandler2->setDefaultAudioConfig();
    TEST_ASSERT_MSG(audioHandler2->isAudioConfigSet() == true, "AudioConfig has not been set. 07");

    TEST_ASSERT_MSG((audioHandler2->getAudioConfiguration() == audioConfig) == false, "AudioConfigs were equal. 08");
}

void TestAudioHandler::testAudioProcessorInterface()
{
    auto audioHandler1 = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::RTAUDIO_WRAPPER);

    ProcessorManager& manager = audioHandler1->getProcessors();

    TEST_ASSERT(manager.addProcessor(AudioProcessorFactory::getAudioProcessor(AudioProcessorFactory::WAV_WRITER, false)));
    TEST_ASSERT(manager.hasAudioProcessor(AudioProcessorFactory::WAV_WRITER));
    TEST_ASSERT(manager.removeAudioProcessor(AudioProcessorFactory::WAV_WRITER));

    TEST_ASSERT(manager.clearAudioProcessors());
    TEST_ASSERT(manager.hasAudioProcessor(AudioProcessorFactory::WAV_WRITER) == false);
    TEST_ASSERT(manager.removeAudioProcessor(AudioProcessorFactory::WAV_WRITER) == false);
}

void TestAudioHandler::testAudioDevices(const std::string processorName)
{
    auto audioHandler1 = AudioHandlerFactory::getAudioHandler(processorName);
    
    if(audioHandler1->getAudioDevices().size() > 0)
    {
        std::vector<std::string> names;
        names.reserve(audioHandler1->getAudioDevices().size());
        for(const AudioDevice& dev : audioHandler1->getAudioDevices())
        {
            TEST_ASSERT(!dev.name.empty());
            TEST_ASSERT_MSG(names.end() == std::find(names.begin(), names.end(), dev.name), "Device name is not unique!");
            names.push_back(dev.name);
        }
    }
}
