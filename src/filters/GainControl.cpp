/* 
 * File:   GainControl.cpp
 * Author: daniel
 * 
 * Created on January 16, 2016, 1:33 PM
 */

#include "filters/GainControl.h"
#include <exception>
#include <cmath>

const double GainControl::SILENCE_THRESHOLD = 0.0;
const Parameter* GainControl::TARGET_GAIN = Parameters::registerParameter(Parameter(ParameterCategory::PROCESSORS, 'g', "gain", "Specifies the target gain", "1.0"));

GainControl::GainControl(const std::string& name) : AudioProcessor(name), gainEnabled(false), gain(1.0), amplifier(nullptr), calculator(nullptr)
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

bool GainControl::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode)
{
    const std::string gainParameter = configMode->getCustomConfiguration(TARGET_GAIN->longName, "Insert gain to apply on the volume", "1.0");
    try
    {
        gain = std::stod(gainParameter);
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
            break;
        case AudioConfiguration::AUDIO_FORMAT_SINT16:
            amplifier = &GainControl::amplify<int16_t>;
            calculator = &GainControl::calculate<int16_t>;
            break;
        case AudioConfiguration::AUDIO_FORMAT_SINT32:
            amplifier = &GainControl::amplify<int32_t>;
            calculator = &GainControl::calculate<int32_t>;
            break;
        case AudioConfiguration::AUDIO_FORMAT_FLOAT32:
            amplifier = &GainControl::amplify<float>;
            calculator = &GainControl::calculate<float>;
            break;
        case AudioConfiguration::AUDIO_FORMAT_FLOAT64:
            amplifier = &GainControl::amplify<double>;
            calculator = &GainControl::calculate<double>;
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
    double gain = calculator(inputBuffer, inputBufferByteSize);
    //convert to dB and check against threshold
    //TODO is wrong?? is always about 1.0 - 1.12
    //XXX better threshold
    if(std::pow(10.0, gain/20.0) <= SILENCE_THRESHOLD)
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

template<typename T>
double GainControl::calculate(void* buffer, const unsigned int bufferSize)
{
    T* buf = (T*) buffer;
    double gain = 0;
    const unsigned int bufSize = bufferSize / sizeof(T);
    for(unsigned int i = 0; i < bufSize; ++i)
    {
        gain += buf[i] < 0 ? -buf[i] : buf[i];
    }
    return gain / bufSize;
}

template<typename T>
void GainControl::amplify(void* buffer, const unsigned int bufferSize, double gain)
{
    T* buf = (T*) buffer;
    const unsigned int bufSize = bufferSize / sizeof(T);
    for(unsigned int i = 0; i < bufSize; ++i)
    {
        //this works, since we read the byte before writing it
        //TODO add checking for overflow -> clip
        //per std::numeric_limits<T>::max() and ::min()
        buf[i] = buf[i] * gain;
    }
}