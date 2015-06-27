#include "AudioProcessor.h"
#include "configuration.h"


AudioProcessor::AudioProcessor(std::string name) : name(name) {}

const std::string AudioProcessor::getName()
{
    return name;
}

void AudioProcessor::setName(std::string name)
{
    this->name = name;
}

bool AudioProcessor::configure(AudioConfiguration config)
{
	//dummy implementation, does nothing
    return true;
}


