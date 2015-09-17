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

const std::vector<std::string> AudioHandlerFactory::allAudioHandlerNames{
#ifdef RTAUDIOWRAPPER_H
    RTAUDIO_WRAPPER
#endif
};