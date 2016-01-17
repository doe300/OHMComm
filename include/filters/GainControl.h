/* 
 * File:   GainControl.h
 * Author: daniel
 *
 * Created on January 16, 2016, 1:33 PM
 */

#ifndef GAINCONTROL_H
#define	GAINCONTROL_H

#include "AudioProcessor.h"
#include "Parameters.h"

/*!
 * Audio-processor which can be used to control the volume
 */
class GainControl : public AudioProcessor
{
public:
    GainControl(const std::string& name);
    
    unsigned int getSupportedAudioFormats() const;
    unsigned int getSupportedSampleRates() const;
    const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;
    PayloadType getSupportedPlayloadType() const;
    
    bool configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode);
    bool cleanUp();
    
    unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
    unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

private:
    typedef void (*Amplifier)(void* buffer, const unsigned int bufferSize, double gain);
    typedef double (*GainCalculator)(void* buffer, const unsigned int bufferSize);
    static const double SILENCE_THRESHOLD;
    static const Parameter* TARGET_GAIN;
    bool gainEnabled;
    double gain;
    Amplifier amplifier;
    GainCalculator calculator;
    
    template<typename T>
    static double calculate(void* buffer, const unsigned int bufferSize);
    
    template<typename T>
    static void amplify(void* buffer, const unsigned int bufferSize, double gain);
};

#endif	/* GAINCONTROL_H */

