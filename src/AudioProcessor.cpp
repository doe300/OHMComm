#include "AudioProcessor.h"

AudioProcessor::AudioProcessor(const std::string name) : name(name)
{
}

const std::string AudioProcessor::getName() const
{
    return name;
}

PayloadType AudioProcessor::getSupportedPlayloadType() const
{
    return PayloadType::ALL;
}

bool AudioProcessor::configure(const AudioConfiguration& config, std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    //dummy implementation, does nothing
    return true;
}

bool AudioProcessor::cleanUp()
{
    //dummy implementation, does nothing
    return true;
}

void AudioProcessor::startup()
{
    //dummy implementation, does nothing
}
