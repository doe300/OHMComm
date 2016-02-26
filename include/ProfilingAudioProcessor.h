#ifndef PROFILINGAUDIOPROCESSOR_H
#define	PROFILINGAUDIOPROCESSOR_H

#include <string>
#include <chrono>

#include "AudioProcessor.h"

/*!
 * AudioProcessor which wraps another processor profiling the method-calls to the AudioProcessor#processInputData()
 * and AudioProcessor#processOutputData() methods
 */
class ProfilingAudioProcessor : public AudioProcessor
{
public:
    ProfilingAudioProcessor(AudioProcessor* profiledProcessor);

    ~ProfilingAudioProcessor();
    /*!
     * Returns the total duration of the AudioProcessor#processInputData() for the profiled AudioProcessor
     */
    unsigned long getTotalInputTime() const;
    /*!
     * Returns the total duration of the AudioProcessor#processOutputData() for the profiled AudioProcessor
     */
    unsigned long getTotalOutputTime() const;
    /*!
     * Returns the total number of tracked method-calls
     */
    unsigned long getTotalCount() const;
    void reset();

    /*!
     * Wraps the configure-method of the profiled processor
     */
    bool configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);

    bool cleanUp();
    PayloadType getSupportedPlayloadType() const;
    void startup();
    unsigned int getSupportedAudioFormats() const;
    const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;
    unsigned int getSupportedSampleRates() const;
    unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);
    unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);
private:
    AudioProcessor* profiledProcessor;
    unsigned long outputProcessingTime;
    unsigned long inputProcessingTime;
    unsigned long count;

    void addTimeInputProcessing(long ms);
    void addTimeOutputProcessing(long ms);
};

#endif
