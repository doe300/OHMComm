#include "AudioProcessor.h"

AudioProcessor::AudioProcessor(const std::string name, const ProcessorCapabilities capabilities) : name(name), capabilities(capabilities)
{
}

const std::string AudioProcessor::getName() const
{
    return name;
}

unsigned int AudioProcessor::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_ALL;
}

unsigned int AudioProcessor::getSupportedSampleRates() const
{
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

const std::vector<int> AudioProcessor::getSupportedBufferSizes(unsigned int sampleRate) const
{
    return {BUFFER_SIZE_ANY};
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

const ProcessorCapabilities& AudioProcessor::getCapabilities() const
{
    return capabilities;
}
