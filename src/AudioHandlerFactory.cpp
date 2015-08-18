#include "AudioHandlerFactory.h"
#include "RTAudioWrapper.h"

//Initialize names
const std::string AudioHandlerFactory::RTAUDIO_WRAPPER = "RtAudioWrapper";

auto AudioHandlerFactory::getAudioHandler(const std::string name, const AudioConfiguration& audioConfig) ->std::unique_ptr<AudioHandler>
{
    #ifdef RTAUDIOWRAPPER_H
    if (name == RTAUDIO_WRAPPER)
    {
        std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper(audioConfig));
        return std::move(rtaudiowrapper);
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
    throw std::invalid_argument("No AudioHandler for this name!");
}

const std::vector<std::string> AudioHandlerFactory::getAudioHandlerNames()
{
    std::vector<std::string> handlerNames;
    #ifdef RTAUDIOWRAPPER_H
    handlerNames.push_back(RTAUDIO_WRAPPER);
    #endif
    return handlerNames;
}

std::string AudioHandlerFactory::getDefaultAudioHandlerName()
{
    #ifdef RTAUDIOWRAPPER_H
    return RTAUDIO_WRAPPER;
    #else
    return getAudioHandlerNames()[0];
    #endif
}
