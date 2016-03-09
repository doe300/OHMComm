/* 
 * File:   GSMCodec.cpp
 * Author: daniel
 * 
 * Created on March 9, 2016, 12:25 PM
 */
#ifdef GSM_HEADER   //Only compile when GSM is linked
#include "codecs/GSMCodec.h"


/* From RFC 3551 section 4.5.8: "
 * Blocks of 160 audio samples are compressed into 33 octets, for an
 * effective data rate of 13,200 b/s."
 */
static constexpr unsigned int GSM_FRAME_SIZE = 33;
static constexpr ProcessorCapabilities gsmCapabilities = {true, false, false, false, false, 0, GSM_FRAME_SIZE * 50 /* 33 Byte times 50 frames per second */};


GSMCodec::GSMCodec(const std::string& name) : AudioProcessor(name, gsmCapabilities), encoder(nullptr), decoder(nullptr)
{
}

GSMCodec::~GSMCodec()
{
    if(encoder != nullptr)
        gsm_destroy(encoder);
    if(decoder != nullptr)
        gsm_destroy(decoder);
}

unsigned int GSMCodec::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

unsigned int GSMCodec::getSupportedSampleRates() const
{
    return AudioConfiguration::SAMPLE_RATE_8000;
}

const std::vector<int> GSMCodec::getSupportedBufferSizes(unsigned int sampleRate) const
{
    //as of RFC3551 we use a default package size of 20ms -> 160 frames at 8kHz sampling rate
    const int defaultPackageSize = 20 * sampleRate / 1000;    //20 ms
    return {defaultPackageSize};
}

PayloadType GSMCodec::getSupportedPlayloadType() const
{
    return PayloadType::GSM;
}

bool GSMCodec::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    encoder = gsm_create();
    decoder = gsm_create();
}

unsigned int GSMCodec::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    //according to source-code, in-line encoding should work
    gsm_encode(encoder, (gsm_signal*)inputBuffer, (gsm_byte*)inputBuffer);
    //now we have a single frame of valid audio-data
    return GSM_FRAME_SIZE;
}

unsigned int GSMCodec::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    //according to source-code, in-line decoding should work
    gsm_decode(decoder, (gsm_byte*)outputBuffer, (gsm_signal*)outputBuffer);
    //according to source-code, always returns zero
    
    //now we have 160 samples of 16bit signed integer of audio-data
    return 160 * sizeof(gsm_signal);
}

bool GSMCodec::cleanUp()
{
    if(encoder != nullptr)
        gsm_destroy(encoder);
    if(decoder != nullptr)
        gsm_destroy(decoder);
    encoder = nullptr;
    decoder = nullptr;
}

#endif