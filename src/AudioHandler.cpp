#include "AudioHandler.h"

AudioHandler::AudioHandler() : audioConfiguration({0})
{
    
}

AudioHandler::~AudioHandler()
{
}

void AudioHandler::printAudioProcessorOrder(std::ostream& OutputStream) const
{
    processors.printAudioProcessorOrder(OutputStream);
}

bool AudioHandler::addProcessor(AudioProcessor *audioProcessor)
{
    return processors.addProcessor(audioProcessor);
}

bool AudioHandler::removeAudioProcessor(AudioProcessor *audioProcessor)
{
    return processors.removeAudioProcessor(audioProcessor);
}

bool AudioHandler::removeAudioProcessor(std::string nameOfAudioProcessor)
{
    return processors.removeAudioProcessor(nameOfAudioProcessor);
}

bool AudioHandler::clearAudioProcessors()
{
    return processors.clearAudioProcessors();
}

bool AudioHandler::hasAudioProcessor(AudioProcessor *audioProcessor) const
{
    return processors.hasAudioProcessor(audioProcessor);
}

bool AudioHandler::hasAudioProcessor(std::string nameOfAudioProcessor) const
{
    return processors.hasAudioProcessor(nameOfAudioProcessor);
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
    if(getAudioDevices()[audioConfiguration.inputDeviceID].isInputDevice())
        mode = (PlaybackMode)(mode | INPUT);
    if(getAudioDevices()[audioConfiguration.outputDeviceID].isOutputDevice())
        mode = (PlaybackMode)(mode | OUTPUT);
    return mode;
}
