
#include "sip/SupportedFormats.h"
#include "AudioProcessorFactory.h"

#include <iostream>

std::vector<SupportedFormat> SupportedFormats::availableFormats = {};

const SupportedFormat* SupportedFormats::OPUS_48000 = SupportedFormats::registerFormat(SupportedFormat(112, "opus", 48000, 2, AudioProcessorFactory::OPUS_CODEC));
const SupportedFormat* SupportedFormats::OPUS_24000 = SupportedFormats::registerFormat(SupportedFormat(113, "opus", 24000, 2, AudioProcessorFactory::OPUS_CODEC));
const SupportedFormat* SupportedFormats::OPUS_16000 = SupportedFormats::registerFormat(SupportedFormat(114, "opus", 16000, 2, AudioProcessorFactory::OPUS_CODEC));
const SupportedFormat* SupportedFormats::OPUS_12000 = SupportedFormats::registerFormat(SupportedFormat(115, "opus", 12000, 2, AudioProcessorFactory::OPUS_CODEC));
const SupportedFormat* SupportedFormats::OPUS_8000 = SupportedFormats::registerFormat(SupportedFormat(116, "opus", 8000, 2, AudioProcessorFactory::OPUS_CODEC));


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
