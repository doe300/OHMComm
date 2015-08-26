#include "ProfilingAudioProcessor.h"

ProfilingAudioProcessor::ProfilingAudioProcessor(AudioProcessor* profiledProcessor) : AudioProcessor(profiledProcessor->getName()),
    profiledProcessor(profiledProcessor), outputProcessingTime(0), inputProcessingTime(0), count(0)
{
}

ProfilingAudioProcessor::~ProfilingAudioProcessor()
{
    delete(profiledProcessor);
}

void ProfilingAudioProcessor::addTimeInputProcessing(long ms)
{
	count++;
	inputProcessingTime += ms;
};

void ProfilingAudioProcessor::addTimeOutputProcessing(long ms)
{
	count++;
	outputProcessingTime += ms;
}

unsigned long ProfilingAudioProcessor::getTotalInputTime() const
{
    return inputProcessingTime;
}

unsigned long ProfilingAudioProcessor::getTotalOutputTime() const
{
    return outputProcessingTime;
}

unsigned long ProfilingAudioProcessor::getTotalCount() const
{
    return count;
}

void ProfilingAudioProcessor::reset()
{
	outputProcessingTime = 0;
	inputProcessingTime = 0;
	count = 0;
}

bool ProfilingAudioProcessor::configure(const AudioConfiguration& audioConfig)
{
    return profiledProcessor->configure(audioConfig);
}

unsigned int ProfilingAudioProcessor::getSupportedAudioFormats() const
{
    return profiledProcessor->getSupportedAudioFormats();
}

const std::vector<int> ProfilingAudioProcessor::getSupportedBufferSizes(unsigned int sampleRate) const
{
    return profiledProcessor->getSupportedBufferSizes(sampleRate);
}

unsigned int ProfilingAudioProcessor::getSupportedSampleRates() const
{
    return profiledProcessor->getSupportedSampleRates();
}

unsigned int ProfilingAudioProcessor::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    auto start = std::chrono::high_resolution_clock::now();
    int retVal = profiledProcessor->processInputData(inputBuffer, inputBufferByteSize, userData);
    auto end = std::chrono::high_resolution_clock::now();
    long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    addTimeInputProcessing(duration);
    return retVal;
}

unsigned int ProfilingAudioProcessor::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    auto start = std::chrono::high_resolution_clock::now();
    int retVal = profiledProcessor->processOutputData(outputBuffer, outputBufferByteSize, userData);
    auto end = std::chrono::high_resolution_clock::now();
    long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    addTimeOutputProcessing(duration);
    return retVal;
}
