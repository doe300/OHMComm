#include "AudioProcessor.h"

AudioProcessor::AudioProcessor(const std::string name) : name(name) {}

const std::string AudioProcessor::getName() const
{
    return name;
}

bool AudioProcessor::configure(const AudioConfiguration& config)
{
    //dummy implementation, does nothing
    return true;
}

bool AudioProcessor::cleanUp()
{
	//dummy implementation, does nothing
	return true;
}
