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
		Statistics::addProcessor(audioProcessor->getName());
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
			Statistics::removeProcessor(audioProcessor->getName());
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
			Statistics::removeProcessor(nameOfAudioProcessor);
            return true; // Successful removed
        }
    }
    return false;
}

auto AudioHandler::clearAudioProcessors() -> bool
{
    audioProcessors.clear();
	Statistics::removeAllProcessors();
    return true;
}

auto AudioHandler::configureAudioProcessors() -> bool
{
    for (AudioProcessor *processor : audioProcessors)
    {
        bool result = processor->configure(audioConfiguration);
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
	
    unsigned int bufferSize = outputBufferByteSize;
    for (auto i = audioProcessors.size(); i > 0; i--)
    {
		auto beginTime = Clock::now();
        bufferSize = audioProcessors.at(i-1)->processOutputData(outputBuffer, bufferSize, streamData);
		auto endTime = Clock::now();
		auto difference = endTime - beginTime;
		int ms = std::chrono::duration_cast<std::chrono::milliseconds> (difference).count();
    }
}

void AudioHandler::processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, StreamData *streamData)
{
    unsigned int bufferSize = inputBufferByteSize;

	for (auto i = 0; i < audioProcessors.size(); i++)
    {
		auto beginTime = Clock::now();
		bufferSize = audioProcessors.at(i)->processInputData(inputBuffer, bufferSize, streamData);
		auto endTime = Clock::now();
		auto difference = endTime - beginTime;
		int ms = std::chrono::duration_cast<std::chrono::milliseconds> (difference).count();
    }
}

auto AudioHandler::isAudioConfigSet() const -> bool
{
    return flagAudioConfigSet;
}

auto AudioHandler::isPrepared() const -> bool
{
    return flagPrepared;
}