/* 
 * File:   Resampler.h
 * Author: daniel
 *
 * Created on March 2, 2016, 4:58 PM
 */

#ifndef RESAMPLER_H
#define	RESAMPLER_H

#include "AudioProcessor.h"

/*!
 * Simple resampler to convert higher sample-rates to lower ones (and back).
 * 
 * This implementation only supports integer conversion rates -> the higher sample-rate must be a integer multiple of the lower one.
 */
class Resampler : public AudioProcessor
{
public:
    Resampler(const std::string& name, const std::vector<unsigned int>& inputSampleRates);
    virtual ~Resampler();

    virtual unsigned int getSupportedAudioFormats() const;

    virtual void configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);

    virtual unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);
    virtual unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);

    virtual bool cleanUp();
    
    /*!
     * Determines the best matching sample-rate from the given vector for the given output-rate
     * 
     * \param availableSampleRates A list of all available input sample-rates
     * 
     * \param outputSampleRate The output sample-rate to match
     * 
     * \return the best matching input sample-rate or zero if no such sample-rate could be found
     */
    static unsigned int getBestInputSampleRate(const std::vector<unsigned int>& availableSampleRates, const unsigned int outputSampleRate);

private:
    typedef unsigned int (*Compressor)(const void* input, void* output, unsigned int numSamples, uint8_t numChannels, uint8_t factor);
    typedef unsigned int (*Extrapolator)(const void* input, void* output, unsigned int numSamples, uint8_t numChannels, uint8_t factor);
    const std::vector<unsigned int> inputSampleRates;
    uint8_t audioFormatSize;
    uint8_t samplesFactor;
    uint8_t numInputChannels;
    uint8_t numOutputChannels;
    Compressor compressFunc;
    Extrapolator extrapolateFunc;
    char* writeBuffer;
    
    template<typename AudioFormat>
    static unsigned int compress(const void* input, void* output, unsigned int numSamples, uint8_t numChannels, uint8_t factor);
    
    template<typename AudioFormat>
    static unsigned int extrapolate(const void* input, void* output, unsigned int numSamples, uint8_t numChannels, uint8_t factor);

};

#endif	/* RESAMPLER_H */

