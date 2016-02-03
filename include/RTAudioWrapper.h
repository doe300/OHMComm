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
    auto prepare(const std::shared_ptr<ConfigurationMode> configMode) -> bool;
    auto getBufferSize() -> unsigned int;

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

    /*!
     * Automatically selects the best audio format out of the supported formats
     */
    auto autoSelectAudioFormat(RtAudioFormat supportedFormats) -> RtAudioFormat;

    /*!
     * Returns the sample-rate as number of the best supported sample-rate flag
     */
    auto autoSelectSampleRate(unsigned int supportedRatesFlag) -> unsigned int;

    /*!
     * "Asks" the AudioProcessors for supported audio-configuration and uses the sample-rate, frame-size and
     * number of samples per package all processors can agree on
     *
     * \return whether all processors could agree on a value for every field
     */
    bool queryProcessorSupport();

    /*!
     * Maps the supported sample-rates from the device to the flags specified in AudioConfiguration
     */
    unsigned int mapDeviceSampleRates(std::vector<unsigned int> sampleRates);

    /*!
     * Returns the best match for the number of buffered frames according to all processors
     */
    unsigned int findOptimalBufferSize(unsigned int defaultBufferSize);
};

#endif
