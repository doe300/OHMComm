#include "AudioProcessor.h"
#include "configuration.h"


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

void AudioProcessor::configure()
{
    //dummy implementation, does nothing
}

