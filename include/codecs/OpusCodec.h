#ifdef OPUS_HEADER //Only compile, if opus is linked
#ifndef OHMCOMM_OPUS_H
#define OHMCOMM_OPUS_H
#include "processors/AudioProcessor.h"
#include OPUS_HEADER

#include <iostream>
namespace ohmcomm
{
    namespace codecs
    {

        class OpusCodec : public AudioProcessor
        {
        public:

            //! constructor, initialises the OpusApplication type and the OpusEncoderObject and OpusDecoderObject
            OpusCodec(const std::string name);

            //! returns supported Audio Formats by Opus: only rtaudio SINT16 or FLOAT32 supported
            unsigned int getSupportedAudioFormats() const override;
            //! returns supported Sample Rates by Opus: Opus supports this sampleRates: 8000, 12000, 16000, 24000, or 48000.
            unsigned int getSupportedSampleRates() const override;

            /*! 
             * returns supported buffer sizes by Opus
             * supported buffer sizes are (2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
             * sample rates/permitted values:
             * 8000:
             * - 20, 40, 80, 160, 320, 480
             * 12000:
             * - 30, 60, 120, 240, 480, 720
             * 16000:
             * - 40, 80, 160, 320, 640, 960
             * 24000:
             * - 60, 120, 240, 480, 960, 1440
             * 48000:
             * - 120, 240, 480, 960, 1920, 2880 
             */
            const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const override;

            /*!
             * \return The OPUS payload-type
             */
            PayloadType getSupportedPlayloadType() const override;

            //! configure the Opus Processor, this creates OpusEncoder and OpusDecoderObject and initializes private variables: outputDeviceChannels, rtaudioFormat and ErrorCode
            void configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize, const ProcessorCapabilities& chainCapabilities) override;

            /*! 
             * encodes the Data in inputBuffer(only Signed 16bit or float 32bit PCM and one frame supported) and writes the encoded Data in inputBuffer
             * supported size of one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
             * returns lenght of encodedPacket in Bytes
             */
            unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData) override;

            /*! 
             * decodes the Data in outputBuffer and writes it to the outputBuffer(only Signed 16bit or float 32bit PCM and one frame supported)
             * supported size of one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
             * returns size of outputBuffer in Bytes
             */
            unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData) override;

            //!destructor: destroys OpusEncoder and OpusDecoderObject
            ~OpusCodec();

        private:
            OpusEncoder *OpusEncoderObject;
            OpusDecoder *OpusDecoderObject;
            bool useFEC;
            //number of outputDeviceChannels needed for the calculation of outputBytes in processOutputData
            unsigned int outputDeviceChannels;
            //RtAudioFormat needed to decide if we need the floating point or the fixed-point implementation of opus-encode/decode
            unsigned long rtaudioFormat;
        };
    }
}
#endif
#endif
