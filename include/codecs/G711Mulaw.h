/* 
 * File:   ProcessorMuLaw.h
 * Author: daniel
 *
 * Created on January 8, 2016, 11:38 AM
 */

#ifndef OHMCOMM_G711MULAW_H
#define	OHMCOMM_G711MULAW_H

#include "processors/AudioProcessor.h"
#include "g711common.h"

namespace ohmcomm
{
    namespace codecs
    {

        /**
         * Implementation of the G.711 mu-law audio-codec
         * 
         * See: https://en.wikipedia.org/wiki/%CE%9C-law_algorithm
         * See: http://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-G.711-198811-I!!PDF-E&type=items
         * 
         * Uses the Payload-type 0 (PCMU), see: https://tools.ietf.org/html/rfc3551 Section 4.5.14
         */
        class G711Mulaw : public AudioProcessor
        {
        public:
            G711Mulaw(const std::string& name);

            ~G711Mulaw();

            unsigned int getSupportedAudioFormats() const;

            unsigned int getSupportedSampleRates() const;

            const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;

            PayloadType getSupportedPlayloadType() const;

            void configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);

            bool cleanUp();

            unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);

            unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

        private:
            int16_t* writeBuffer;
            uint16_t maxBufferSize;
        };
    }
}
#endif	/* OHMCOMM_G711MULAW_H */

