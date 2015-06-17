#ifndef RTAUDIOWRAPPER_H

#define	RTAUDIOWRAPPER_H

#include "RtAudio.h"
#include "AudioHandler.h"
#include "math.h" // ceiling
#include <memory> // unique_ptr
#include <string.h> //memcpy

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // semaphores
#else
/* TODO, linux semaphores needed */
#endif

/*!
 * Implementation of AudioIO wrapping the RtAudio-library
 */
class RtAudioWrapper : public AudioHandler
{
public:
	RtAudioWrapper();
	RtAudioWrapper(const AudioConfiguration &audioConfig);

    /* deny copies with the copy constructor */
    RtAudioWrapper(const RtAudioWrapper & copy) = delete;

    /* AudioIO methods */
    void startRecordingMode();
    void startPlaybackMode();
    void startDuplexMode();
    void setConfiguration(const AudioConfiguration &audioConfiguration);
    /* suspends the stream */
    void suspend();
    /* resume the stream (only possible if the stream was not stopped) */
    void resume();
    /* close the whole communication process */
    void stop();
    /* stop() and resets audioConfiguration */
    void reset();
	/* sets default audio config */
	void setDefaultAudioConfig();
    /* This is a blocking function, do NOT call this function within the RtAudio callback */
    void playData(void *playbackData, unsigned int size);
	auto prepare() -> bool;
	auto getBufferSize() -> unsigned int;

    /* Callbacks */
    auto callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData) -> int;
    static auto callbackHelper(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *rtAudioWrapperObject) -> int;

private:
    /* variables for the "void playData(...)" function
     * to enable synchronizing between rtaudio- and main-thread */
    #ifdef _WIN32
    HANDLE semaphore_waitForMainThread;
    HANDLE semaphore_waitForWorkerThread;
    #else
    /* TODO, two linux semaphores needed */
    #endif
    void *bufferAudioOutput;
    bool BufferAudioOutHasData = false;


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

    /*!
     * Automatically selects the best audio format out of the supported formats
     */
    auto autoSelectAudioFormat(RtAudioFormat supportedFormats) -> RtAudioFormat;

	void playbackFileData(void *outputBuffer);
};

#endif