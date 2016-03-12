#include "audio/AudioHandler.h"

using namespace ohmcomm;

AudioHandler::AudioHandler() : audioConfiguration({0})
{
    
}

AudioHandler::~AudioHandler()
{
}

void AudioHandler::start(const PlaybackMode mode)
{
    processors.startupAudioProcessors();
    startHandler(mode);
}

ProcessorManager& AudioHandler::getProcessors()
{
    return processors;
}

AudioConfiguration AudioHandler::getAudioConfiguration()
{
    return this->audioConfiguration;
}

bool AudioHandler::isAudioConfigSet() const
{
    return flagAudioConfigSet;
}

bool AudioHandler::isPrepared() const
{
    return flagPrepared;
}

AudioHandler::PlaybackMode AudioHandler::getMode()
{
    if(!flagAudioConfigSet || !flagPrepared)
        return UNDEFINED;
    PlaybackMode mode = UNDEFINED;
    if(audioConfiguration.inputDeviceID != AudioConfiguration::INVALID_DEVICE && getAudioDevices()[audioConfiguration.inputDeviceID].isInputDevice())
        mode = (PlaybackMode)(mode | INPUT);
    if(audioConfiguration.outputDeviceID != AudioConfiguration::INVALID_DEVICE && getAudioDevices()[audioConfiguration.outputDeviceID].isOutputDevice())
        mode = (PlaybackMode)(mode | OUTPUT);
    return mode;
}
