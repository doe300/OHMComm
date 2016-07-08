#include "audio/AudioHandlerFactory.h"
#include "audio/RTAudioWrapper.h"
#include "audio/PortAudioWrapper.h"

using namespace ohmcomm;

//Initialize names
const std::string AudioHandlerFactory::RTAUDIO_WRAPPER = "RtAudio";
const std::string AudioHandlerFactory::PORTAUDIO_WRAPPER = "PortAudio";

auto AudioHandlerFactory::getAudioHandler(const std::string name, const AudioConfiguration& audioConfig) ->std::unique_ptr<AudioHandler>
{
    #ifdef RTAUDIOWRAPPER_H
    if (name == RTAUDIO_WRAPPER)
    {
        std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper(audioConfig));
        return std::move(rtaudiowrapper);
    }
    #endif
    #ifdef PORTAUDIOWRAPPER_H
    if (name == PORTAUDIO_WRAPPER)
    {
        std::unique_ptr<PortAudioWrapper> wrapper(new PortAudioWrapper(audioConfig));
        return std::move(wrapper);
    }
    #endif
    throw std::invalid_argument("No AudioHandler for this name!");
}

auto AudioHandlerFactory::getAudioHandler(const std::string name) ->std::unique_ptr<AudioHandler>
{
    #ifdef RTAUDIOWRAPPER_H
    if (name == RTAUDIO_WRAPPER)
    {
        std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper);
        return std::move(rtaudiowrapper);
    }
    #endif
    #ifdef PORTAUDIOWRAPPER_H
    if (name == PORTAUDIO_WRAPPER)
    {
        std::unique_ptr<PortAudioWrapper> wrapper(new PortAudioWrapper);
        return std::move(wrapper);
    }
    #endif
    throw std::invalid_argument("No AudioHandler for this name!");
}

std::vector<std::string> generateAudioHandlerNames()
{
    std::vector<std::string> names(2);
    #ifdef RTAUDIOWRAPPER_H
    names.push_back(AudioHandlerFactory::RTAUDIO_WRAPPER);
    #endif
    #ifdef PORTAUDIOWRAPPER_H
    names.push_back(AudioHandlerFactory::PORTAUDIO_WRAPPER);
    #endif
    return names;
}

const std::vector<std::string> AudioHandlerFactory::allAudioHandlerNames = generateAudioHandlerNames();