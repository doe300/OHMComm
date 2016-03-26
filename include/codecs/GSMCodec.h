/* 
 * File:   GSMCodec.h
 * Author: daniel
 *
 * Created on March 9, 2016, 12:25 PM
 */

#ifdef GSM_HEADER   //Only compile when GSM is linked
#ifndef GSMCODEC_H
#define	GSMCODEC_H

#include GSM_HEADER
#include "processors/AudioProcessor.h"

namespace ohmcomm
{
    namespace codecs
    {

        /*!
         * Codec using the GSM 06.10 standard available here: http://www.quut.com/gsm/
         * 
         * RTP payload for GSM is defined in RFC 3551
         */
        class GSMCodec : public AudioProcessor
        {
        public:
            GSMCodec(const std::string& name);
            virtual ~GSMCodec();

            virtual unsigned int getSupportedAudioFormats() const override;

            virtual unsigned int getSupportedSampleRates() const override;

            virtual const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const override;

            virtual PayloadType getSupportedPlayloadType() const override;

            virtual void configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize, const ProcessorCapabilities& chainCapabilities) override;

            virtual unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData) override;

            virtual unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData) override;

            virtual bool cleanUp() override;

        private:
            gsm encoder;
            gsm decoder;
        };
    }
}
#endif	/* GSMCODEC_H */
#endif