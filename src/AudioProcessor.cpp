#include "AudioProcessor.h"


AudioProcessor::AudioProcessor(std::string name) : name(name) {}

/*
 * region: getters and setters
 */
auto AudioProcessor::getName() const -> std::string
{
	return name;
}

void AudioProcessor::setName(std::string name)
{
	this->name = name;
}


