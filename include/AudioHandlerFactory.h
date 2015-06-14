#ifndef AUDIOHANDLERFACTORY
#define	AUDIOHANDLERFACTORY

#include "AudioHandlerFactory.h"
#include "RTAudioWrapper.h"
#include <locale>

class AudioHandlerFactory
{
public:
	static auto getAudioHandler(std::string name, AudioConfiguration &audioConfig)->std::unique_ptr<AudioHandler>;
	static auto getAudioHandler(std::string name) -> std::unique_ptr<AudioHandler>;
private:
	static auto stringToUpperCase(const std::string& s)->std::string;
};

#endif