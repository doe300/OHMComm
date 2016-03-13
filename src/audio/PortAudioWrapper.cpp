/* 
 * File:   PortAudioWrapper.cpp
 * Author: daniel
 * 
 * Created on February 10, 2016, 1:21 PM
 */
#ifdef PORTAUDIO_HEADER //Only compile, if PortAudio is linked
#include <exception>
#include <string.h>

#include "audio/PortAudioWrapper.h"

using namespace ohmcomm;

PortAudioWrapper::PortAudioWrapper() : streamData(new StreamData())
{
    throwOnError(Pa_Initialize());
}

PortAudioWrapper::PortAudioWrapper(const AudioConfiguration &audioConfig) : PortAudioWrapper()
{
    setConfiguration(audioConfig);
}

PortAudioWrapper::~PortAudioWrapper()
{
    delete streamData;
    Pa_Terminate();
}

void PortAudioWrapper::startHandler(const PlaybackMode mode)
{
    if(flagPrepared)
    {
        PaStreamParameters* input = (mode & PlaybackMode::INPUT) == PlaybackMode::INPUT ? &inputParams : nullptr;
        PaStreamParameters* output = (mode & PlaybackMode::OUTPUT) == PlaybackMode::OUTPUT ? &outputParams : nullptr;
        streamStartTime = 0;
        Pa_OpenStream(&stream, input, output, audioConfiguration.sampleRate, streamData->nBufferFrames, 0, &PortAudioWrapper::callbackHelper, this);
        resume();
    }
    else
    {
        std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
    }
}

void PortAudioWrapper::setConfiguration(const AudioConfiguration& audioConfiguration)
{
    this->audioConfiguration = audioConfiguration;
    flagAudioConfigSet = true;
}

void PortAudioWrapper::suspend()
{
    if(stream != nullptr && throwOnError(Pa_IsStreamActive(stream)))
    {
        throwOnError(Pa_StopStream(stream));
    }
}

void PortAudioWrapper::resume()
{
    if(stream != nullptr && throwOnError(Pa_IsStreamStopped(stream)))
    {
        throwOnError(Pa_StartStream(stream));
    }
}

void PortAudioWrapper::stop()
{
    suspend();
    if(stream != nullptr)
    {
        throwOnError(Pa_CloseStream(stream));
    }
    processors.cleanUpAudioProcessors();
}

void PortAudioWrapper::reset()
{
    stop();
    audioConfiguration = { 0 };
    flagAudioConfigSet = false;
    flagPrepared = false;
}

void PortAudioWrapper::setDefaultAudioConfig()
{
    AudioConfiguration audioConfig{};
    audioConfig.inputDeviceID = Pa_GetDefaultInputDevice();
    audioConfig.outputDeviceID = Pa_GetDefaultOutputDevice();

    audioConfig.inputDeviceChannels = 2;
    audioConfig.outputDeviceChannels = 2;
    audioConfig.audioFormatFlag = 0;
    audioConfig.sampleRate = 0;
    audioConfig.framesPerPackage = 0;

    setConfiguration(audioConfig);
}

bool PortAudioWrapper::prepare(const std::shared_ptr<ConfigurationMode> configMode)
{
    std::cout << "Using PortAudio in version: " << Pa_GetVersionText() << std::endl;
    std::cout << "\tDefault Host API: " << Pa_GetHostApiInfo(Pa_GetDefaultHostApi())->name << std::endl;
    
    /* If there is no configuration set, then load the default */
    if (flagAudioConfigSet == false)
        setDefaultAudioConfig();
    
    if(audioConfiguration.forceAudioFormatFlag == 0)
    {
        //PortAudio doesn't support float64, so disable it
        //by forcing "everything but float64"
        audioConfiguration.forceAudioFormatFlag = AudioConfiguration::AUDIO_FORMAT_ALL ^ AudioConfiguration::AUDIO_FORMAT_FLOAT64;
    }
    //checks if there is a configuration all processors support
    if(!processors.queryProcessorSupport(audioConfiguration, getAudioDevices()[audioConfiguration.inputDeviceID]))
    {
        std::cerr << "AudioProcessors could not agree on configuration!" << std::endl;
        return false;
    }
    
    bool resultA = this->initStreamParameters();
    bufferSize = audioConfiguration.framesPerPackage * Pa_GetSampleSize(inputParams.sampleFormat) * inputParams.channelCount;
    bool resultB = processors.configureAudioProcessors(audioConfiguration, configMode, bufferSize);

    if (resultA && resultB) {
        this->flagPrepared = true;
        inputBuffer.reserve(bufferSize);
        return true;
    }

    return false;
}

const std::vector<AudioDevice>& PortAudioWrapper::getAudioDevices()
{
    static std::vector<AudioDevice> devices{};
    
    if(devices.empty())
    {
        const PaDeviceIndex defaultOutputDeviceIndex = Pa_GetDefaultOutputDevice();
        const PaDeviceIndex defaultInputDeviceIndex = Pa_GetDefaultInputDevice();
        for(PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); ++i)
        {
            const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
            if(info != nullptr)
            {
                devices.push_back({info->name, (unsigned int)info->maxOutputChannels, (unsigned int)info->maxInputChannels, 
                        i == defaultOutputDeviceIndex, i == defaultInputDeviceIndex, 
                        0 /* no way to simply get native formats */, true, getSupportedSampleRates(i, *info)});
            }
        }
    }
    
    return devices;
}

std::vector<unsigned int> PortAudioWrapper::getSupportedSampleRates(const PaDeviceIndex deviceIndex, const PaDeviceInfo& deviceInfo)
{
    //available sample-rates (according to AudioConfiguration) are
    const static std::vector<unsigned int> allSampleRates = {8000, 12000, 16000, 24000, 32000, 44100, 48000, 96000, 192000};
    const static PaStreamParameters inputParams = {deviceIndex, 2, paInt16, 0, nullptr};
    const static PaStreamParameters outputParams = {deviceIndex, 2, paInt16, 0, nullptr};
    //for correct retrieval of input/output only rates
    const PaStreamParameters* input = deviceInfo.maxInputChannels > 0 ? &inputParams : nullptr;
    const PaStreamParameters* output = deviceInfo.maxOutputChannels > 0 ? &outputParams : nullptr;
    std::vector<unsigned int> supportedSampleRates{};
    for(unsigned int sampleRate : allSampleRates)
    {
        if(Pa_IsFormatSupported(input, output, sampleRate) == 0)
        {
            supportedSampleRates.push_back(sampleRate);
        }
    }
    return supportedSampleRates;
}

int PortAudioWrapper::callbackHelper(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
    PortAudioWrapper* wrapper = static_cast <PortAudioWrapper*>(userData);
    return wrapper->callback(input, output, frameCount, timeInfo->currentTime, statusFlags);
}

int PortAudioWrapper::callback(const void* inputBuffer, void* outputBuffer, unsigned long frameCount, const double streamTime, PaStreamCallbackFlags statusFlags)
{
    printf("%d %d\n", frameCount, audioConfiguration.framesPerPackage);
    if(statusFlags == paInputOverflow)
    {
        std::cout << "Overflow\n";
    }
    if(statusFlags == paOutputUnderflow)
    {
        std::cout << "Underflow\n";
    }
    
    if(streamStartTime == 0)
    {
        //PortAudio uses a clock with unknown origin, so we must define our own start
        streamStartTime = streamTime;
    }
    streamData->nBufferFrames = frameCount;
    //streamTime is the number of seconds since start of stream, so we convert to number of microseconds
    streamData->streamTime = lround((streamTime - streamStartTime) * 1000000);
    Statistics::setCounter(Statistics::TOTAL_ELAPSED_MILLISECONDS, (streamTime - streamStartTime) * 1000);
    Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_RECORDED, bufferSize);
    Statistics::incrementCounter(Statistics::COUNTER_FRAMES_RECORDED, frameCount);
    Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_OUTPUT, bufferSize);
    Statistics::incrementCounter(Statistics::COUNTER_FRAMES_OUTPUT, frameCount);

    //reset maximum size, in case a processor illegally modifies it
    this->streamData->maxBufferSize = bufferSize;
    this->streamData->isSilentPackage = false;
    if (inputBuffer != nullptr)
    {
        //PortAudio input-buffer is read-only, so use own internal buffer
        const unsigned int inputBufferSize = frameCount * inputParams.channelCount * throwOnError(Pa_GetSampleSize(inputParams.sampleFormat));
        memcpy(&this->inputBuffer[0], inputBuffer, inputBufferSize);
        //FIXME fails when used with Opus (opus throws playback-error OPUS_BAD_ARG)
        //fails too with any other codec expecting a fixed number of frames in the first package
        //reason: in the first callback, the buffer is not fully filled -> results in a sample-number/size not supported by opus
        //generally, PortAudio does not adhere to the buffer-size and fills it only as much as it deems necessary (which doesn't work for us)
        processors.processAudioInput(&this->inputBuffer[0], inputBufferSize, streamData);
    }

    //reset maximum size, in case a processor illegally modifies it
    this->streamData->maxBufferSize = bufferSize;
    if (outputBuffer != nullptr)
        processors.processAudioOutput(outputBuffer, bufferSize, streamData);

    return paContinue;
}

bool PortAudioWrapper::initStreamParameters()
{
    //XXX to solve bug with any codec expecting constant buffer size (too small buffer size), we set the latency to the buffer-size
    const double optimalLatency = audioConfiguration.framesPerPackage/(double)audioConfiguration.sampleRate;
    
    inputParams.channelCount = audioConfiguration.inputDeviceChannels;
    inputParams.device = audioConfiguration.inputDeviceID;
    inputParams.sampleFormat = mapSampleFormat(audioConfiguration.audioFormatFlag);
    inputParams.suggestedLatency = optimalLatency; //Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    outputParams.channelCount = audioConfiguration.outputDeviceChannels;
    outputParams.device = audioConfiguration.outputDeviceID;
    outputParams.sampleFormat = mapSampleFormat(audioConfiguration.audioFormatFlag);
    outputParams.suggestedLatency = optimalLatency; //Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;
    
    //check for supported parameters depends on the playback-mode
    const PaStreamParameters* input = Pa_GetDeviceInfo(inputParams.device)->maxInputChannels > 0 ? &inputParams : nullptr;
    const PaStreamParameters* output = Pa_GetDeviceInfo(outputParams.device)->maxOutputChannels > 0 ? &outputParams : nullptr;
    return throwOnError(Pa_IsFormatSupported(input, output, audioConfiguration.sampleRate)) == 0;
}

PaSampleFormat PortAudioWrapper::mapSampleFormat(const unsigned int sampleFormatFlag)
{
    if ((sampleFormatFlag & AudioConfiguration::AUDIO_FORMAT_FLOAT32) == AudioConfiguration::AUDIO_FORMAT_FLOAT32)
    {
        return paFloat32; 
    }
    if ((sampleFormatFlag & AudioConfiguration::AUDIO_FORMAT_SINT32) == AudioConfiguration::AUDIO_FORMAT_SINT32)
    {
        return paInt32;
    }
    if ((sampleFormatFlag & AudioConfiguration::AUDIO_FORMAT_SINT24) == AudioConfiguration::AUDIO_FORMAT_SINT24)
    {
        return paInt24;
    }
    if ((sampleFormatFlag & AudioConfiguration::AUDIO_FORMAT_SINT16) == AudioConfiguration::AUDIO_FORMAT_SINT16)
    {
        return paInt16;
    }
    //fall back to worst quality
    return paInt8;
}
#endif /* PORTAUDIO_HEADER */