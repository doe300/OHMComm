
#include "sip/SupportedFormats.h"
#include "AudioProcessorFactory.h"

#include <iostream>

const std::string SupportedFormat::MEDIA_LPCM("LPCM");
const std::string SupportedFormat::MEDIA_PCMA("PCMA");
const std::string SupportedFormat::MEDIA_PCMU("PCMU");

const std::string SupportedFormat::FORMAT_OPUS_DTX("usedtx");
const std::string SupportedFormat::FORMAT_OPUS_FEC("useinbandfec");

std::vector<SupportedFormat> SupportedFormats::availableFormats = {};

#ifdef OPUS_HEADER
const SupportedFormat* SupportedFormats::OPUS_48000 = SupportedFormats::registerFormat(SupportedFormat(PayloadType::OPUS, "opus", 48000, 2, AudioProcessorFactory::OPUS_CODEC, false, std::string(SupportedFormat::FORMAT_OPUS_DTX).append("=1; ").append(SupportedFormat::FORMAT_OPUS_FEC).append("=0")));
#endif
const SupportedFormat* SupportedFormats::G711_PCMA = SupportedFormats::registerFormat(SupportedFormat(PayloadType::PCMA, SupportedFormat::MEDIA_PCMA, 8000, 1, AudioProcessorFactory::G711_PCMA, true));
const SupportedFormat* SupportedFormats::G711_PCMU = SupportedFormats::registerFormat(SupportedFormat(PayloadType::PCMU, SupportedFormat::MEDIA_PCMU, 8000, 1, AudioProcessorFactory::G711_PCMU, true));
const SupportedFormat* SupportedFormats::L16_2_44100 = SupportedFormats::registerFormat(SupportedFormat(PayloadType::L16_2, SupportedFormat::MEDIA_LPCM, 44100, 2, "", true));

const SupportedFormat* SupportedFormats::registerFormat(SupportedFormat&& format)
{
    std::vector<SupportedFormat>::iterator pos = availableFormats.begin();
    while(pos != availableFormats.end())
    {
        if((*pos).payloadType == format.payloadType)
        {
            std::cerr << "Format already registered for payload-type " << format.payloadType << ": " << (*pos).encoding << "/" << (*pos).sampleRate << std::endl;
            return nullptr;
        }
        ++pos;
    }
    availableFormats.push_back(std::move(format));
    return &(*pos);
}

const std::vector<SupportedFormat> SupportedFormats::getFormats()
{
    return availableFormats;
}

const SupportedFormat* SupportedFormats::getFormat(const int payloadType)
{
    for(const SupportedFormat& format : availableFormats)
    {
        if(format.payloadType == payloadType)
        {
            return &format;
        }
    }
    return nullptr;
}
