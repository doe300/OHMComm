
#include "sip/SupportedFormats.h"
#include "AudioProcessorFactory.h"
#include "rtp/RTPPackageHandler.h"
#include "sip/SDPMessageHandler.h"

#include <iostream>

std::vector<SupportedFormat> SupportedFormats::availableFormats = {};

const SupportedFormat* SupportedFormats::OPUS_48000 = SupportedFormats::registerFormat(SupportedFormat(PayloadType::OPUS, "opus", 48000, 2, AudioProcessorFactory::OPUS_CODEC));
const SupportedFormat* SupportedFormats::L16_2_44100 = SupportedFormats::registerFormat(SupportedFormat(PayloadType::L16_2, MediaDescription::MEDIA_PCM, 44100, 2, "", true));

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
