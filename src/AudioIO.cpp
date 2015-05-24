#include "AudioIO.h"


void AudioIO::printAudioProcessorOrder() const
{
	for (const auto processor : audioProcessors)
	{
		std::cout << processor->getName() << std::endl;
	}
}

void AudioIO::addProcessor(AudioProcessor *audioProcessor)
{
	if (hasAudioProcessor(audioProcessor) == false)
		audioProcessors.push_back(audioProcessor);
}

void AudioIO::removeAudioProcessor(AudioProcessor *audioProcessor)
{
	for (size_t i = 0; i < audioProcessors.size(); i++)
	{
		if ( (audioProcessors.at(i))->getName() == audioProcessor->getName() )
			audioProcessors.erase(audioProcessors.begin() + i);
	}
}

void AudioIO::removeAudioProcessor(std::string nameOfAudioProcessor)
{
	for (size_t i = 0; i < audioProcessors.size(); i++)
	{
		if (audioProcessors.at(i)->getName() == nameOfAudioProcessor)
			audioProcessors.erase(audioProcessors.begin() + i);
	}
}

void AudioIO::clearAudioProcessors()
{
	audioProcessors.clear();
}

void AudioIO::configureAudioProcessors()
{
    for (AudioProcessor *processor : audioProcessors)
    {
        processor->configure();
    }
}

auto AudioIO::hasAudioProcessor(AudioProcessor *audioProcessor) const -> bool
{
	for (const auto processor : audioProcessors)
	{
		if ( processor->getName() == audioProcessor->getName() )
			return true;
	}
	return false;
}

auto AudioIO::hasAudioProcessor(std::string nameOfAudioProcessor) const -> bool
{
	for (const auto processor : audioProcessors)
	{
		if ( processor->getName() == nameOfAudioProcessor )
			return true;
	}
	return false;
}

void AudioIO::processAudioOutput(void *outputBuffer, const unsigned int &outputBufferByteSize, void *userData1, void *userData2)
{
	for (auto i = audioProcessors.size(); i > 0; i--)
	{
		audioProcessors.at(i-1)->processOutputData(outputBuffer, outputBufferByteSize);
	}
}

void AudioIO::processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, void *userData1, void *userData2)
{
	for (auto processor : audioProcessors)
	{
		processor->processInputData(inputBuffer, inputBufferByteSize);
	}
}