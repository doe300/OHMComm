
#ifdef RTAUDIO_HEADER //Only compile, if RtAudio is linked
#ifndef RTAUDIOWRAPPER_H
#define	RTAUDIOWRAPPER_H

#include RTAUDIO_HEADER
#include "AudioHandler.h"
#include "math.h" // ceiling
#include <memory> // unique_ptr
#include <string.h> //memcpy

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // semaphores
#endif
namespace ohmcomm
{

    /*!
     * Implementation of AudioIO wrapping the RtAudio-library
     */
    class RtAudioWrapper : public AudioHandler
    {
    public:
        RtAudioWrapper();
        RtAudioWrapper(const AudioConfiguration &audioConfig);

        ~RtAudioWrapper();

        /* deny copies with the copy constructor */
        RtAudioWrapper(const RtAudioWrapper & copy) = delete;

        /* AudioIO methods */
        void setConfiguration(const AudioConfiguration &audioConfiguration) override;
        /* suspends the stream */
        void suspend() override;
        /* resume the stream (only possible if the stream was not stopped) */
        void resume() override;
        /* close the whole communication process */
        void stop() override;
        /* stop() and resets audioConfiguration */
        void reset() override;
        /* sets default audio config */
        void setDefaultAudioConfig() override;
        bool prepare(const std::shared_ptr<ConfigurationMode> configMode) override;

        const std::vector<AudioDevice>& getAudioDevices() override;

        /* Callbacks */
        auto callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData) -> int;
        static auto callbackHelper(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *rtAudioWrapperObject) -> int;

    private:
        void *bufferAudioOutput;


        RtAudio rtaudio;
        RtAudio::StreamParameters input, output;

        unsigned int outputBufferByteSize = 0;
        unsigned int inputBufferByteSize = 0;

        StreamData *streamData;

        /* preparing for a openstream call */
        auto initRtAudioStreamParameters() -> bool;

        /* returns the size of a frame in bytes */
        auto getAudioFormatByteSize(RtAudioFormat rtaudioFormat) -> int;

        /* returns the actual output framesize in bytes */
        auto getOutputFrameSize() -> int;

        /* returns the actual input framesize in bytes */
        auto getInputFrameSize() -> int;

        void startHandler(const PlaybackMode mode) override;
    };
}
#endif	/* RTAUDIOWRAPPER_H */
#endif /* RTAUDIO_HEADER */