/* 
 * File:   PortAudioWrapper.h
 * Author: daniel
 *
 * Created on February 10, 2016, 1:21 PM
 */

#ifdef PORTAUDIO_HEADER //Only compile, if PortAudio is linked
#ifndef PORTAUDIOWRAPPER_H
#define	PORTAUDIOWRAPPER_H

#include <thread>

#include "AudioHandler.h"
#include PORTAUDIO_HEADER
namespace ohmcomm
{

    /*!
     * Audio-library wrapper for PortAudio - http://portaudio.com/
     * 
     * This implementation uses a extra thread with blocking I/O on the audio-stream.
     * 
     * Although a callback-implementation is possible (as it was before), PortAudio doesn't guarantee a steady
     * number of samples per call to the callback, which is required by some codecs and audio-processors.
     * So the implementation uses blocking I/O which guarantees the exact number of samples per call to the audio-processors.
     */
    class PortAudioWrapper : public AudioHandler
    {
    public:
        PortAudioWrapper();
        PortAudioWrapper(const AudioConfiguration &audioConfig);

        ~PortAudioWrapper();

        /* deny copies with the copy constructor */
        PortAudioWrapper(const PortAudioWrapper & copy) = delete;

        void setConfiguration(const AudioConfiguration &audioConfiguration);
        void suspend();
        void resume();
        void stop();
        void reset();
        void setDefaultAudioConfig();
        auto prepare(const std::shared_ptr<ConfigurationMode> configMode) -> bool;

        const std::vector<AudioDevice>& getAudioDevices();
    private:
        StreamData* streamData;
        PaStreamParameters outputParams;
        PaStreamParameters inputParams;
        PaStream* stream;
        unsigned int inputBufferSize;
        unsigned int outputBufferSize;
        PaTime streamStartTime = 0;
        PlaybackMode mode;
        std::thread audioThread;

        static inline unsigned int throwOnError(PaError error)
        {
            //error-codes are all negative
            if (error < 0)
                throw std::runtime_error(Pa_GetErrorText(error));
            return (unsigned int) error;
        }

        static std::vector<unsigned int> getSupportedSampleRates(const PaDeviceIndex deviceIndex, const PaDeviceInfo& deviceInfo);

        int callback(void *inputBuffer, void *outputBuffer, unsigned long frameCount, const double streamTime, PaStreamCallbackFlags statusFlags);

        bool initStreamParameters();

        static PaSampleFormat mapSampleFormat(const unsigned int sampleFormatFlag);

        void startHandler(const PlaybackMode mode);
        
        void audioLoop();
    };
}
#endif	/* PORTAUDIOWRAPPER_H */
#endif /* PORTAUDIO_HEADER */