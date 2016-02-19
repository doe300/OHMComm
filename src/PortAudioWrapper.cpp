/* 
 * File:   PortAudioWrapper.cpp
 * Author: daniel
 * 
 * Created on February 10, 2016, 1:21 PM
 */
#ifdef PORTAUDIO_HEADER //Only compile, if PortAudio is linked
#include <exception>
#include <string.h>

#include "PortAudioWrapper.h"

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

void PortAudioWrapper::startRecordingMode()
{
    if(flagPrepared)
    {
        streamStartTime = 0;
        Pa_OpenStream(&stream, &inputParams, nullptr, audioConfiguration.sampleRate, streamData->nBufferFrames, 0, &PortAudioWrapper::callbackHelper, this);
        resume();
    }
    else
    {
        std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
    }
}

void PortAudioWrapper::startPlaybackMode()
{
    if(flagPrepared)
    {
        streamStartTime = 0;
        Pa_OpenStream(&stream, nullptr, &outputParams, audioConfiguration.sampleRate, streamData->nBufferFrames, 0, &PortAudioWrapper::callbackHelper, this);
        resume();
    }
    else
    {
        std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
    }
}

void PortAudioWrapper::startDuplexMode()
{
    if(flagPrepared)
    {
        streamStartTime = 0;
        Pa_OpenStream(&stream, &inputParams, &outputParams, audioConfiguration.sampleRate, streamData->nBufferFrames, 0, &PortAudioWrapper::callbackHelper, this);
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
    cleanUpAudioProcessors();
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
    if(!queryProcessorSupport())
    {
        std::cerr << "AudioProcessors could not agree on configuration!" << std::endl;
        return false;
    }
    
    bool resultA = this->initStreamParameters();
    bool resultB = this->configureAudioProcessors(configMode);

    if (resultA && resultB) {
        this->flagPrepared = true;
        bufferSize = audioConfiguration.framesPerPackage * Pa_GetSampleSize(inputParams.sampleFormat) * inputParams.channelCount;
        inputBuffer.reserve(bufferSize);
        return true;
    }

    return false;
}

unsigned int PortAudioWrapper::getBufferSize()
{
    return bufferSize;
}

const std::vector<AudioHandler::AudioDevice>& PortAudioWrapper::getAudioDevices()
{
    static std::vector<AudioHandler::AudioDevice> devices{};
    
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
                        0 /* no way to simply get native formats */, getSupportedSampleRates(i)});
            }
        }
    }
    
    return devices;
}

std::vector<unsigned int> PortAudioWrapper::getSupportedSampleRates(const PaDeviceIndex deviceIndex)
{
    //available sample-rates (according to AudioConfiguration) are
    const static std::vector<unsigned int> allSampleRates = {8000, 12000, 16000, 24000, 32000, 44100, 48000, 96000, 192000};
    const static PaStreamParameters inputParams = {deviceIndex, 2, paInt16, 0, nullptr};
    const static PaStreamParameters outputParams = {deviceIndex, 2, paInt16, 0, nullptr};
    std::vector<unsigned int> supportedSampleRates{};
    for(unsigned int sampleRate : allSampleRates)
    {
        if(Pa_IsFormatSupported(&inputParams, &outputParams, sampleRate) == 0)
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
        memcpy(&this->inputBuffer[0], inputBuffer, bufferSize);
        this->processAudioInput(&this->inputBuffer[0], bufferSize, streamData);
    }

    //reset maximum size, in case a processor illegally modifies it
    this->streamData->maxBufferSize = bufferSize;
    if (outputBuffer != nullptr)
        this->processAudioOutput(outputBuffer, bufferSize, streamData);

    return paContinue;
}

bool PortAudioWrapper::initStreamParameters()
{
    inputParams.channelCount = audioConfiguration.inputDeviceChannels;
    inputParams.device = audioConfiguration.inputDeviceID;
    inputParams.sampleFormat = mapSampleFormat(audioConfiguration.audioFormatFlag);
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    outputParams.channelCount = audioConfiguration.outputDeviceChannels;
    outputParams.device = audioConfiguration.outputDeviceID;
    outputParams.sampleFormat = mapSampleFormat(audioConfiguration.audioFormatFlag);
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;
    
    return throwOnError(Pa_IsFormatSupported(&inputParams, &outputParams, audioConfiguration.sampleRate)) == 0;
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