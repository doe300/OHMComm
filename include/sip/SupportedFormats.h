/* 
 * File:   SupportedFormats.h
 * Author: daniel
 *
 * Created on December 10, 2015, 11:19 AM
 */

#ifndef SUPPORTEDFORMATS_H
#define	SUPPORTEDFORMATS_H

#include <string>
#include <vector>

namespace ohmcomm
{
    namespace sip
    {

        struct SupportedFormat
        {
            // media name for LPCM samples
            static const std::string MEDIA_LPCM;
            //media name for G.711 A-law samples
            static const std::string MEDIA_PCMA;
            //media name for G.711 mu-law samples
            static const std::string MEDIA_PCMU;
            //media name for GSM 06.10 samples
            static const std::string MEDIA_GSM;

            //parameter name for opus DTX
            static const std::string FORMAT_OPUS_DTX;
            //parameter name for opus FEC
            static const std::string FORMAT_OPUS_FEC;

            const unsigned int payloadType;
            const std::string encoding;
            const unsigned int sampleRate;
            const unsigned short numChannels;
            const std::string processorName;
            const bool isDefaultFormat;
            const std::string parameterLine;

            SupportedFormat(const unsigned int payloadType, const std::string encoding, const unsigned int sampleRate, const unsigned short numChannels,
                            const std::string processorName, const bool defaultFormat = false, const std::string parameterLine = "") :
            payloadType(payloadType), encoding(encoding), sampleRate(sampleRate), numChannels(numChannels), processorName(processorName), isDefaultFormat(defaultFormat), parameterLine(parameterLine)
            {
            }

            SupportedFormat(const SupportedFormat& other) = default;
            SupportedFormat(SupportedFormat&& other) = default;
        };

        class SupportedFormats
        {
        public:
            /*!
             * This method can be used to add processor-specific formats to the list of available supported formats.
             * This method must be called in a static context. For that, it must be used to initialize a static constant value.
             * 
             * NOTE: The order the formats are registered using this function will determine their priority in SDP (first registered, highest priority)
             * 
             * \param format The new supported format to add. This should be a newly created SupportedFormat-object
             * 
             * \return A pointer to the registered format, nullptr otherwise
             */
            static const SupportedFormat* registerFormat(SupportedFormat&& format);

            static const std::vector<SupportedFormat>& getFormats();

            static const SupportedFormat* getFormat(const int payloadType);

        private:
            // A list of all available formats
            static std::vector<SupportedFormat> availableFormats;

            //just for registration
#ifdef OPUS_HEADER //only make opus available, if codec is included
            //as of RFC 7587, the opus format must be opus/48000/2
            static const SupportedFormat* OPUS_48000;
#endif
#ifdef ILBC_HEADER
            static const SupportedFormat* iLBC;
#endif
#ifdef AMR_ENCODER_HEADER
            static const SupportedFormat* AMR_NB;
#endif
            static const SupportedFormat* G711_PCMA;
            static const SupportedFormat* G711_PCMU;
#ifdef GSM_HEADER
            static const SupportedFormat* GSM;
#endif
            static const SupportedFormat* L16_2_44100;
        };
    }
}

#endif	/* SUPPORTEDFORMATS_H */

