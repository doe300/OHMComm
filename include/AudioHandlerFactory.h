#ifndef AUDIOHANDLERFACTORY
#define	AUDIOHANDLERFACTORY

#include "RTAudioWrapper.h"
#include <memory> // unique_ptr
#include <locale>
#include <vector>

/*!
 * Factory-class to provide an audio-handler object without needing to know the details of the implementation.
 */
class AudioHandlerFactory
{
public:
    static const std::string RTAUDIO_WRAPPER;
    static auto getAudioHandler(const std::string name, const AudioConfiguration& audioConfig)->std::unique_ptr<AudioHandler>;
    static auto getAudioHandler(const std::string name) -> std::unique_ptr<AudioHandler>;
    static auto getAudioHandlerNames() -> const std::vector<std::string>;
    static std::string getDefaultAudioHandlerName();
};

#endif