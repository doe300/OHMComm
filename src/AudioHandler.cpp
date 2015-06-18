#include "AudioHandler.h"


void AudioHandler::printAudioProcessorOrder(std::ostream& OutputStream) const
{
	for (const auto processor : audioProcessors)
	{
		OutputStream << processor->getName() << std::endl;
	}
}

auto AudioHandler::addProcessor(AudioProcessor *audioProcessor) -> bool
{
	if (hasAudioProcessor(audioProcessor) == false) {
		audioProcessors.push_back(audioProcessor);
		return true; // Successful added
	}
	return false;
}

auto AudioHandler::removeAudioProcessor(AudioProcessor *audioProcessor) -> bool
{
	for (size_t i = 0; i < audioProcessors.size(); i++)
	{
		if ((audioProcessors.at(i))->getName() == audioProcessor->getName()) {
			audioProcessors.erase(audioProcessors.begin() + i);
			return true; // Successful removed
		}	
	}
	return false;
}

auto AudioHandler::removeAudioProcessor(std::string nameOfAudioProcessor) -> bool
{
	for (size_t i = 0; i < audioProcessors.size(); i++)
	{
		if (audioProcessors.at(i)->getName() == nameOfAudioProcessor) {
			audioProcessors.erase(audioProcessors.begin() + i);
			return true; // Successful removed
		}
	}
	return false;
}

auto AudioHandler::clearAudioProcessors() -> bool
{
	audioProcessors.clear();
	return true;
}

auto AudioHandler::configureAudioProcessors() -> bool
{
    for (AudioProcessor *processor : audioProcessors)
    {
        bool result = processor->configure();
		if (result == false) // Configuration failed
			return false;
    }
	return true;
}

auto AudioHandler::hasAudioProcessor(AudioProcessor *audioProcessor) const -> bool
{
	for (const auto processor : audioProcessors)
	{
		if ( processor->getName() == audioProcessor->getName() )
			return true;
	}
	return false;
}

auto AudioHandler::hasAudioProcessor(std::string nameOfAudioProcessor) const -> bool
{
	for (const auto processor : audioProcessors)
	{
		if ( processor->getName() == nameOfAudioProcessor )
			return true;
	}
	return false;
}

auto AudioHandler::getAudioConfiguration()->AudioConfiguration
{
	return this->audioConfiguration;
}

void AudioHandler::processAudioOutput(void *outputBuffer, const unsigned int &outputBufferByteSize, StreamData *streamData)
{
	for (auto i = audioProcessors.size(); i > 0; i--)
	{
		audioProcessors.at(i-1)->processOutputData(outputBuffer, outputBufferByteSize, streamData);
	}
}

void AudioHandler::processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, StreamData *streamData)
{
	for (auto processor : audioProcessors)
	{
		processor->processInputData(inputBuffer, inputBufferByteSize, streamData);
	}
}

auto AudioHandler::isAudioConfigSet() const -> bool
{
	return isAudioConfigSet;
}

auto AudioHandler::isPrepared() const -> bool
{
	return isPrepared;
}