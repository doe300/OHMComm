#include "AudioHandler.h"

AudioHandler::AudioHandler() : audioConfiguration({0})
{
    
}

AudioHandler::~AudioHandler()
{
}

void AudioHandler::printAudioProcessorOrder(std::ostream& OutputStream) const
{
    for (const auto& processor : audioProcessors)
    {
        OutputStream << processor->getName() << std::endl;
    }
}

bool AudioHandler::addProcessor(AudioProcessor *audioProcessor)
{
    if (hasAudioProcessor(audioProcessor) == false) {
        audioProcessors.push_back(std::unique_ptr<AudioProcessor>(audioProcessor));
        if(ProfilingAudioProcessor* profiler = dynamic_cast<ProfilingAudioProcessor*>(audioProcessor))
        {
            //if this is a profiling processor, we need to add it to statistics to be printed on exit
            Statistics::addProfiler(profiler);
        }
        return true; // Successful added
    }
    return false;
}

bool AudioHandler::removeAudioProcessor(AudioProcessor *audioProcessor)
{
    for (size_t i = 0; i < audioProcessors.size(); i++)
    {
        if ((audioProcessors.at(i))->getName() == audioProcessor->getName()) {
            audioProcessors.erase(audioProcessors.begin() + i);
            if(ProfilingAudioProcessor* profiler = dynamic_cast<ProfilingAudioProcessor*>(audioProcessor))
            {
                Statistics::removeProfiler(profiler);
            }
            return true; // Successful removed
        }	
    }
    return false;
}

bool AudioHandler::removeAudioProcessor(std::string nameOfAudioProcessor)
{
    for (size_t i = 0; i < audioProcessors.size(); i++)
    {
        if (audioProcessors.at(i)->getName() == nameOfAudioProcessor)
        {
            if(ProfilingAudioProcessor* profiler = dynamic_cast<ProfilingAudioProcessor*>(audioProcessors.at(i).get()))
            {
                Statistics::removeProfiler(profiler);
            }
            audioProcessors.erase(audioProcessors.begin() + i);
            return true; // Successful removed
        }
    }
    return false;
}

bool AudioHandler::clearAudioProcessors()
{
    audioProcessors.clear();
    Statistics::removeAllProfilers();
    return true;
}

bool AudioHandler::configureAudioProcessors(const std::shared_ptr<ConfigurationMode> configMode)
{
    for (const auto& processor : audioProcessors)
    {
        std::cout << "Configuring audio-processor '" << processor->getName() << "'..." << std::endl;
        bool result = processor->configure(audioConfiguration, configMode);
        if (result == false) // Configuration failed
            return false;
    }
    return true;
}

bool AudioHandler::cleanUpAudioProcessors()
{
	for (const auto& processor : audioProcessors)
	{
		bool result = processor->cleanUp();
		if (result == false) // Cleanup failed
			return false;
	}
	return true;
}

bool AudioHandler::hasAudioProcessor(AudioProcessor *audioProcessor) const
{
    for (const auto& processor : audioProcessors)
    {
        if ( processor->getName() == audioProcessor->getName() )
            return true;
    }
    return false;
}

bool AudioHandler::hasAudioProcessor(std::string nameOfAudioProcessor) const
{
    for (const auto& processor : audioProcessors)
    {
        if ( processor->getName() == nameOfAudioProcessor )
            return true;
    }
    return false;
}

AudioConfiguration AudioHandler::getAudioConfiguration()
{
    return this->audioConfiguration;
}

void AudioHandler::processAudioOutput(void *outputBuffer, const unsigned int &outputBufferByteSize, StreamData *streamData)
{
    unsigned int bufferSize = outputBufferByteSize;
    for (unsigned int i = audioProcessors.size(); i > 0; i--)
    {
        bufferSize = audioProcessors.at(i-1)->processOutputData(outputBuffer, bufferSize, streamData);
    }
}

void AudioHandler::processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, StreamData *streamData)
{
    unsigned int bufferSize = inputBufferByteSize;
    for (unsigned int i = 0; i < audioProcessors.size(); i++)
    {
        bufferSize = audioProcessors.at(i)->processInputData(inputBuffer, bufferSize, streamData);
    }
}

bool AudioHandler::isAudioConfigSet() const
{
    return flagAudioConfigSet;
}

bool AudioHandler::isPrepared() const
{
    return flagPrepared;
}

unsigned int AudioHandler::autoSelectAudioFormat(unsigned int supportedFormats)
{
    if ((supportedFormats & AudioConfiguration::AUDIO_FORMAT_FLOAT64) == AudioConfiguration::AUDIO_FORMAT_FLOAT64)
    {
        return AudioConfiguration::AUDIO_FORMAT_FLOAT64;
    }
    if ((supportedFormats & AudioConfiguration::AUDIO_FORMAT_FLOAT32) == AudioConfiguration::AUDIO_FORMAT_FLOAT32)
    {
        return AudioConfiguration::AUDIO_FORMAT_FLOAT32; 
    }
    if ((supportedFormats & AudioConfiguration::AUDIO_FORMAT_SINT32) == AudioConfiguration::AUDIO_FORMAT_SINT32)
    {
        return AudioConfiguration::AUDIO_FORMAT_SINT32;
    }
    if ((supportedFormats & AudioConfiguration::AUDIO_FORMAT_SINT24) == AudioConfiguration::AUDIO_FORMAT_SINT24)
    {
        return AudioConfiguration::AUDIO_FORMAT_SINT24;
    }
    if ((supportedFormats & AudioConfiguration::AUDIO_FORMAT_SINT16) == AudioConfiguration::AUDIO_FORMAT_SINT16)
    {
        return AudioConfiguration::AUDIO_FORMAT_SINT16;
    }
    //fall back to worst quality
    return AudioConfiguration::AUDIO_FORMAT_SINT8;
}

unsigned int AudioHandler::autoSelectSampleRate(unsigned int supportedRatesFlag)
{
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_192000) == AudioConfiguration::SAMPLE_RATE_192000)
    {
        return 192000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_96000) == AudioConfiguration::SAMPLE_RATE_96000)
    {
        return 96000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_48000) == AudioConfiguration::SAMPLE_RATE_48000)
    {
        return 48000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_44100) == AudioConfiguration::SAMPLE_RATE_44100)
    {
        return 44100;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_32000) == AudioConfiguration::SAMPLE_RATE_32000)
    {
        return 32000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_24000) == AudioConfiguration::SAMPLE_RATE_24000)
    {
        return 24000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_16000) == AudioConfiguration::SAMPLE_RATE_16000)
    {
        return 16000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_12000) == AudioConfiguration::SAMPLE_RATE_12000)
    {
        return 12000;
    }
    //fall back to worst sample-rate
    return 8000;
}

bool AudioHandler::queryProcessorSupport()
{
    //preset supported audio-formats with device-supported formats
    //although there is a value for native audio-formats in the device-info, RtAudio documentation says:
    // "However, RtAudio will automatically provide format conversion if a particular format is not natively supported."
    unsigned int supportedFormats = AudioConfiguration::AUDIO_FORMAT_ALL;
    if(audioConfiguration.forceAudioFormatFlag != 0)
    {
        //force the specific audio-format
        supportedFormats = audioConfiguration.forceAudioFormatFlag;
    }
    //preset supported sample-rates with device-supported rates
    unsigned int supportedSampleRates = mapDeviceSampleRates(getAudioDevices()[audioConfiguration.inputDeviceID].sampleRates);
    if(audioConfiguration.forceSampleRate != 0)
    {
        //force the specific sample-rate
        supportedSampleRates = mapDeviceSampleRates({audioConfiguration.forceSampleRate});
    }
    for (unsigned int i = 0; i < audioProcessors.size(); i++)
    {
        // a & b return all bits set in a AND b -> all flags supported by both
        supportedFormats = supportedFormats & audioProcessors.at(i)->getSupportedAudioFormats();
        supportedSampleRates = supportedSampleRates &audioProcessors.at(i)->getSupportedSampleRates();
    }
    if(supportedFormats == 0)
    {
        //there is no format supported by all processors
        std::cerr << "Could not find a single audio-format supported by all processors!" << std::endl;
        return false;
    }
    if(supportedSampleRates == 0)
    {
        //there is no sample-rate supported by all processors
        std::cerr << "Could not find a single sample-rate supported by all processors!" << std::endl;
        return false;
    }
    audioConfiguration.audioFormatFlag = autoSelectAudioFormat(supportedFormats);
    audioConfiguration.sampleRate = autoSelectSampleRate(supportedSampleRates);
    
    //find common supported buffer-size, defaults to 512
    int supportedBufferSize = findOptimalBufferSize(512);
    if(supportedBufferSize == 0)
    {
        std::cerr << "Could not find a single buffer-size supported by all processors!" << std::endl;
        return false;
    }
    audioConfiguration.framesPerPackage = supportedBufferSize;
    
    std::cout << "Using audio-format: " << AudioConfiguration::getAudioFormatDescription(audioConfiguration.audioFormatFlag, false) << std::endl;
    std::cout << "Using a sample-rate of " << audioConfiguration.sampleRate << " Hz" << std::endl;
    std::cout << "Using a buffer-size of " << supportedBufferSize << " samples (" << (supportedBufferSize * 1000 / audioConfiguration.sampleRate) << " ms)" << std::endl;
    
    return true;
}

unsigned int AudioHandler::mapDeviceSampleRates(std::vector<unsigned int> sampleRates)
{
    unsigned int sampleRate;
    unsigned int sampleRatesFlags = 0;
    for (unsigned int i = 0; i < sampleRates.size(); i++)
    {
        sampleRate = sampleRates.at(i);
        if(sampleRate == 8000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_8000;
        else if(sampleRate == 12000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_12000;
        else if(sampleRate == 16000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_16000;
        else if(sampleRate == 24000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_24000;
        else if(sampleRate == 32000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_32000;
        else if(sampleRate == 44100)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_44100;
        else if(sampleRate == 48000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_48000;
        else if(sampleRate == 96000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_96000;
        else if(sampleRate == 192000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_192000;
        else
            std::cout << "Unrecognized sample-rate: " << sampleRate << std::endl;
    }
    
    return sampleRatesFlags;
}

unsigned int AudioHandler::findOptimalBufferSize(unsigned int defaultBufferSize)
{
    //get all supported buffer-sizes
    std::vector<std::vector<int>> processorBufferSizes(audioProcessors.size());
    for (unsigned int i = 0; i < audioProcessors.size(); i++)
    {
        processorBufferSizes[i] = audioProcessors[i]->getSupportedBufferSizes(audioConfiguration.sampleRate);
    }
    //number of processors supporting all buffer-sizes
    std::vector<bool> processorSupportAll(processorBufferSizes.size(), false);
    //iterate through all priorities and check if the value is supported by other processors
    for(unsigned int sizeIndex = 0; sizeIndex < 32; sizeIndex ++)
    {
        for(unsigned int procIndex = 0; procIndex < processorBufferSizes.size(); procIndex ++)
        {
            //processor has no size-entries left
            if(processorBufferSizes[procIndex].size() <= sizeIndex)
            {
                continue;
            }
            if(processorBufferSizes[procIndex][sizeIndex] == AudioProcessor::BUFFER_SIZE_ANY)
            {
                processorSupportAll[procIndex] = true;
                continue;
            }
            //"suggest" buffer-size
            int suggestedBufferSize = processorBufferSizes[procIndex][sizeIndex];
            //check "suggested" buffer-size against all other processors
            bool suggestionAccepted = true;
            for(unsigned int checkProcIndex = 0; checkProcIndex < processorBufferSizes.size(); checkProcIndex ++)
            {
                bool singleProcessorAccepted = false;
                //we already checked all sizes below sizeIndex
                for(unsigned int checkSizeIndex = sizeIndex; checkSizeIndex < processorBufferSizes[checkProcIndex].size(); checkSizeIndex ++)
                {
                    if(processorBufferSizes[checkProcIndex][checkSizeIndex] == suggestedBufferSize || processorBufferSizes[checkProcIndex][checkSizeIndex] == AudioProcessor::BUFFER_SIZE_ANY)
                    {
                        singleProcessorAccepted = true;
                        //break;
                    }
                }
                if(!singleProcessorAccepted)
                {
                    suggestionAccepted = false;
                    //break;
                }
            }
            if(suggestionAccepted)
            {
                return suggestedBufferSize;
            }
        }
    }
    //if all processors only have BUFFER_SIZE_ALL, return default-value
    bool supportAll = true;
    for(unsigned int procIndex = 0; procIndex < processorSupportAll.size(); procIndex ++)
    {
        if(!processorSupportAll[procIndex])
        {
            supportAll = false;
            break;
        }
    }
    if(supportAll)
    {
        return defaultBufferSize;
    }
    
    return 0;
}