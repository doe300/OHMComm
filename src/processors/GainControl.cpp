/* 
 * File:   GainControl.cpp
 * Author: daniel
 * 
 * Created on January 16, 2016, 1:33 PM
 */

#include "processors/GainControl.h"
#include <exception>
#include <cmath>
#include <limits>

//silence threshold is at -30dB (since -40dB seems to be absolute silence)
//Opus uses a lower silence-threshold
const double GainControl::SILENCE_THRESHOLD = fromDB(-30);
const Parameter* GainControl::TARGET_GAIN = Parameters::registerParameter(Parameter(ParameterCategory::PROCESSORS, 'g', "gain", "Specifies the amplification, in dB", "0"));
double GainControl::lowerSampleLimit = std::numeric_limits<double>::max();
double GainControl::upperSampleLimit = std::numeric_limits<double>::min();

static constexpr ProcessorCapabilities gainCapabilities = {false, true, false, false, false, 0, 0};

GainControl::GainControl(const std::string& name) : AudioProcessor(name, gainCapabilities), gainEnabled(false), gain(1.0), amplifier(nullptr), calculator(nullptr)
{
}

unsigned int GainControl::getSupportedAudioFormats() const
{
    //we support every format except 24 bit signed integer, because there is no such native data type
    //since we don't know the size (in bytes) of float and double, we need to check these here
    return AudioConfiguration::AUDIO_FORMAT_SINT8 | AudioConfiguration::AUDIO_FORMAT_SINT16 | AudioConfiguration::AUDIO_FORMAT_SINT32 | 
            (sizeof(float) == 4 ? AudioConfiguration::AUDIO_FORMAT_FLOAT32 : 0) | (sizeof(double) == 8 ? AudioConfiguration::AUDIO_FORMAT_FLOAT64 : 0);
}

unsigned int GainControl::getSupportedSampleRates() const
{
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

const std::vector<int> GainControl::getSupportedBufferSizes(unsigned int sampleRate) const
{
    return {BUFFER_SIZE_ANY};
}

PayloadType GainControl::getSupportedPlayloadType() const
{
    return PayloadType::ALL;
}

bool GainControl::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    const std::string gainParameter = configMode->getCustomConfiguration(TARGET_GAIN->longName, "Insert gain to apply on the volume", "0.0");
    numInputChannels = audioConfig.inputDeviceChannels;
    try
    {
        gain = fromDB(std::stod(gainParameter));
        //if the gain is too marginal, don't do anything
        gainEnabled = (gain > 1.0 ? gain - 1.0 : 1.0 - gain) > 0.05;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
        //fetches errors on converting input to double
        gainEnabled = false;
        return false;
    }
    //calculate amplifier-function to use
    switch(audioConfig.audioFormatFlag)
    {
        case AudioConfiguration::AUDIO_FORMAT_SINT8:
            amplifier = &GainControl::amplify<int8_t>;
            calculator = &GainControl::calculate<int8_t>;
            upperSampleLimit = std::numeric_limits<int8_t>::max() / gain;
            lowerSampleLimit = std::numeric_limits<int8_t>::min() / gain;
            break;
        case AudioConfiguration::AUDIO_FORMAT_SINT16:
            amplifier = &GainControl::amplify<int16_t>;
            calculator = &GainControl::calculate<int16_t>;
            upperSampleLimit = std::numeric_limits<int16_t>::max() / gain;
            lowerSampleLimit = std::numeric_limits<int16_t>::min() / gain;
            break;
        case AudioConfiguration::AUDIO_FORMAT_SINT32:
            amplifier = &GainControl::amplify<int32_t>;
            calculator = &GainControl::calculate<int32_t>;
            upperSampleLimit = std::numeric_limits<int32_t>::max() / gain;
            lowerSampleLimit = std::numeric_limits<int32_t>::min() / gain;
            break;
        case AudioConfiguration::AUDIO_FORMAT_FLOAT32:
            amplifier = &GainControl::amplify<float>;
            calculator = &GainControl::calculate<float>;
            upperSampleLimit = std::numeric_limits<float>::max() / gain;
            lowerSampleLimit = std::numeric_limits<float>::min() / gain;
            break;
        case AudioConfiguration::AUDIO_FORMAT_FLOAT64:
            amplifier = &GainControl::amplify<double>;
            calculator = &GainControl::calculate<double>;
            upperSampleLimit = std::numeric_limits<double>::max() / gain;
            lowerSampleLimit = std::numeric_limits<double>::min() / gain;
            break;
    default:
        amplifier = nullptr;
        calculator = nullptr;
        gainEnabled = false;
    }
    if(gainEnabled)
    {
        std::cout << "Gain Control: Using gain of " << gain << std::endl;
    }
    return true;
}

bool GainControl::cleanUp()
{
    return true;
}

unsigned int GainControl::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    double gain = calculator(inputBuffer, inputBufferByteSize, numInputChannels);
    if(gain <= SILENCE_THRESHOLD)
    {
        userData->isSilentPackage = true;
    }
    return inputBufferByteSize;
}

unsigned int GainControl::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    if(gainEnabled && !userData->isSilentPackage)
    {
        //don't amplify silent package -> there is no use: x * 0 = 0
        amplifier(outputBuffer, outputBufferByteSize, gain);
    }
    return outputBufferByteSize;
}

template<typename AudioFormat>
double GainControl::calculate(void* buffer, const unsigned int bufferSize, const unsigned char numChannels)
{
    //see https://stackoverflow.com/questions/4152201/calculate-decibels
    //see https://stackoverflow.com/questions/13734710/is-there-a-way-get-something-like-decibel-levels-from-an-audio-file-and-transfor
    AudioFormat* buf = (AudioFormat*) buffer;
    double gain = 0;
    double sampleGain = 0;
    const unsigned int bufSize = bufferSize / sizeof(AudioFormat);
    for(unsigned int i = 0; i < bufSize; i += numChannels)
    {
        sampleGain = 1;
        for(unsigned char c = 0; c < numChannels; ++c)
        {
            sampleGain *= buf[i] < 0 ? -buf[i] : buf[i];
        }
        gain +=sampleGain;
    }
    //return root mean square of all samples
    return sqrt(gain / (bufSize / numChannels));
}

template<typename AudioFormat>
void GainControl::amplify(void* buffer, const unsigned int bufferSize, double gain)
{
    AudioFormat* buf = (AudioFormat*) buffer;
    const unsigned int bufSize = bufferSize / sizeof(AudioFormat);
    for(unsigned int i = 0; i < bufSize; ++i)
    {
        //this works, since we read the byte before writing it
        buf[i] = clipOverflow(buf[i], gain);
    }
}

template<typename AudioFormat>
AudioFormat GainControl::clipOverflow(AudioFormat sample, double gain)
{
    if(sample == 0)
    {
        // 0 * x = 0
        return sample;
    }
    if(gain <= 1.0)
    {
        //if we're reducing the signal, overflow can't occur
        return sample * gain;
    }
    if(sample > 0)
    {
        //if we have a positive sample-value, we must test for upper limits
        if(upperSampleLimit > sample)
        {
            //if max/g > s -> s * g > max -> overflow
            return std::numeric_limits<AudioFormat>::max();
        }
    }
    else
    {
        //if we have a negative sample-value, we must test for lower limits
        if(lowerSampleLimit > sample)
        {
            //if min/g > s -> s * g < min -> overflow
            return std::numeric_limits<AudioFormat>::min();
        }
    }
    return sample * gain;
}
