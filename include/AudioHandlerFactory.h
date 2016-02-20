#ifndef AUDIOHANDLERFACTORY
#define	AUDIOHANDLERFACTORY

#include "AudioHandler.h"

#include <memory> // unique_ptr
#include <locale>
#include <vector>

/*!
 * Factory-class to provide an audio-handler object without needing to know the details of the implementation.
 */
class AudioHandlerFactory
{
public:
    /*! Name for the RtAudioWrapper */
    static const std::string RTAUDIO_WRAPPER;
    /*! name for the PortAudioWrapper */
    static const std::string PORTAUDIO_WRAPPER;

    /*!
     * \param name The name of the audio-handler to create
     * \param audioConfig The audio-configuration to initialize the handler with
     * \return an unique-pointer to the created handler
     * \throw an invalid-argument exception, if there was no audio-handler for the given name
     */
    static auto getAudioHandler(const std::string name, const AudioConfiguration& audioConfig)->std::unique_ptr<AudioHandler>;
    /*!
     * \param name The name of the audio-handler to create
     * \return an unique-pointer to the created handler
     * \throw an invalid-argument exception, if there was no audio-handler for the given name
     */
    static auto getAudioHandler(const std::string name) -> std::unique_ptr<AudioHandler>;

    /*!
     * \return a list of all available audio-handler names
     */
    static inline const std::vector<std::string> getAudioHandlerNames()
    {
        return allAudioHandlerNames;
    }

    /*!
     * \return the name of the default audio-handler
     */
    static inline const std::string getDefaultAudioHandlerName()
    {
        return allAudioHandlerNames[0];
    }
    
private:
    //! A list of all registered audio-handlers, the first audio-handler is used as default
    static const std::vector<std::string>allAudioHandlerNames;
};

#endif
