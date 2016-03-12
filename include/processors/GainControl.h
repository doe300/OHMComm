/* 
 * File:   GainControl.h
 * Author: daniel
 *
 * Created on January 16, 2016, 1:33 PM
 */

#ifndef GAINCONTROL_H
#define	GAINCONTROL_H

#include <cmath>

#include "processors/AudioProcessor.h"
#include "Parameters.h"

namespace ohmcomm
{

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

        void configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);
        bool cleanUp();

        unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
        unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

    private:
        typedef void (*Amplifier)(void* buffer, const unsigned int bufferSize, double gain);
        typedef double (*GainCalculator)(void* buffer, const unsigned int bufferSize, const unsigned char numChannels);
        static const double SILENCE_THRESHOLD;
        static const Parameter* TARGET_GAIN;
        unsigned char numInputChannels;
        bool gainEnabled;
        double gain;
        Amplifier amplifier;
        GainCalculator calculator;
        //we use double, because it is the largest type used
        static double upperSampleLimit;
        static double lowerSampleLimit;

        template<typename AudioFormat>
        static double calculate(void* buffer, const unsigned int bufferSize, const unsigned char numChannels);

        template<typename AudioFormat>
        static void amplify(void* buffer, const unsigned int bufferSize, double gain);

        template<typename AudioFormat>
        static AudioFormat clipOverflow(AudioFormat sample, double gain);

        inline static double todB(double value)
        {
            return 20 * (std::log(value) / std::log(10));
        }

        inline static double fromDB(double dBValue)
        {
            return std::pow(10, dBValue / 20);
        }
    };
}
#endif	/* GAINCONTROL_H */

