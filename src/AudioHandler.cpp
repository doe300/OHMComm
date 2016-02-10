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