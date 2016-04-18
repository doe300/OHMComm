/* 
 * File:   TestAudioProcessors.cpp
 * Author: daniel
 * 
 * Created on September 23, 2015, 5:26 PM
 */

#include "TestAudioProcessors.h"

using namespace ohmcomm;

TestAudioProcessors::TestAudioProcessors()
{
#ifdef OPUS_HEADER
    TEST_ADD_WITH_STRING(TestAudioProcessors::testAudioProcessorConfiguration, AudioProcessorFactory::OPUS_CODEC);
#endif
    TEST_ADD_WITH_STRING(TestAudioProcessors::testAudioProcessorConfiguration, AudioProcessorFactory::WAV_WRITER);
    TEST_ADD_WITH_STRING(TestAudioProcessors::testAudioProcessorConfiguration, AudioProcessorFactory::G711_PCMA);
    TEST_ADD_WITH_STRING(TestAudioProcessors::testAudioProcessorConfiguration, AudioProcessorFactory::G711_PCMU);
#ifdef ILBC_HEADER
    TEST_ADD_WITH_STRING(TestAudioProcessors::testAudioProcessorConfiguration, AudioProcessorFactory::ILBC_CODEC);
#endif
}

void TestAudioProcessors::testAudioProcessorConfiguration(const std::string processorName)
{
    AudioProcessor* proc = AudioProcessorFactory::getAudioProcessor(processorName, false);
    
    TEST_ASSERT_EQUALS(processorName, proc->getName());
    TEST_ASSERT_MSG(proc->getSupportedAudioFormats()!= 0, "No audio-formats supported!");
    TEST_ASSERT_MSG(proc->getSupportedSampleRates() != 0, "No sample-rates supported!");
    
    for(const unsigned int& sampleRate : getSampleRates(proc->getSupportedSampleRates()))
    {
        const std::vector<int> bufferSizes = proc->getSupportedBufferSizes(sampleRate);
        TEST_ASSERT_MSG(!bufferSizes.empty(), "Processor does not list any buffer-sizes for supported sample-rate");
    }
    
    TEST_ASSERT_MSG(proc->getSupportedPlayloadType() == PayloadType::ALL || proc->getSupportedPlayloadType() >= 0, "Invalid payload-type!");
    
    if(proc->getCapabilities().supportedResampleRates != 0)
    {
        TEST_ASSERT_MSG(AudioConfiguration::flagToSampleRate(proc->getCapabilities().supportedResampleRates) != 0, "Invalid sample-rates supported!");
    }
    delete proc;
}

std::vector<unsigned int> TestAudioProcessors::getSampleRates(unsigned int supportedRatesFlag)
{
    std::vector<unsigned int> sampleRates{};
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_192000) == AudioConfiguration::SAMPLE_RATE_192000)
    {
        sampleRates.push_back(192000);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_96000) == AudioConfiguration::SAMPLE_RATE_96000)
    {
        sampleRates.push_back(96000);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_48000) == AudioConfiguration::SAMPLE_RATE_48000)
    {
        sampleRates.push_back(48000);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_44100) == AudioConfiguration::SAMPLE_RATE_44100)
    {
        sampleRates.push_back(44100);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_32000) == AudioConfiguration::SAMPLE_RATE_32000)
    {
        sampleRates.push_back(32000);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_24000) == AudioConfiguration::SAMPLE_RATE_24000)
    {
        sampleRates.push_back(24000);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_16000) == AudioConfiguration::SAMPLE_RATE_16000)
    {
        sampleRates.push_back(16000);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_12000) == AudioConfiguration::SAMPLE_RATE_12000)
    {
        sampleRates.push_back(12000);
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_8000) == AudioConfiguration::SAMPLE_RATE_8000)
    {
        sampleRates.push_back(8000);
    }
    return sampleRates;
}