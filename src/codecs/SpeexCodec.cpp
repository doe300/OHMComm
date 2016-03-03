/* 
 * File:   SpeexCodec.cpp
 * Author: daniel
 * 
 * Created on March 2, 2016, 3:01 PM
 */
#ifdef SPEEX_HEADER //Only compile if speex is linked
#include "codecs/SpeexCodec.h"
#include "Parameters.h"

//TODO not yet tested since sample rates do not match
//TODO we could support stereo (or at least automatic downmixing) via the SPEEX_STEREO_HEADER
//TODO could use speex own resampler (not yet available in my version) to support multiple sample-rates

SpeexCodec::SpeexCodec(const std::string& name) : AudioProcessor(name), speexEncoder(nullptr), speexDecoder(nullptr)
{
}

SpeexCodec::~SpeexCodec()
{
    if(speexEncoder != nullptr)
    {
        speex_encoder_destroy(speexEncoder);
        speex_bits_destroy(&encoderBits);
    }
    if(speexDecoder != nullptr)
    {
        speex_decoder_destroy(speexDecoder);
        speex_bits_destroy(&decoderBits);
    }
}

unsigned int SpeexCodec::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

unsigned int SpeexCodec::getSupportedSampleRates() const
{
    //from manual: "Speex is mainly designed for three different sampling rates: 8 kHz, 16 kHz, and 32 kHz."
    return AudioConfiguration::SAMPLE_RATE_8000|AudioConfiguration::SAMPLE_RATE_16000|AudioConfiguration::SAMPLE_RATE_32000;
}

const std::vector<int> SpeexCodec::getSupportedBufferSizes(unsigned int sampleRate) const
{
    int frameSize;
    switch(sampleRate)
    {
        //narrow band
        case AudioConfiguration::SAMPLE_RATE_8000:
            speex_mode_query(&speex_nb_mode, SPEEX_MODE_FRAME_SIZE, &frameSize);
            break;
        //wide band
        case AudioConfiguration::SAMPLE_RATE_16000:
            speex_mode_query(&speex_wb_mode, SPEEX_MODE_FRAME_SIZE, &frameSize);
            break;
        //ultra-wide band
        case AudioConfiguration::SAMPLE_RATE_32000:
            speex_mode_query(&speex_uwb_mode, SPEEX_MODE_FRAME_SIZE, &frameSize);
            break;
    }
    //the returned frame-size corresponds to 20ms frame-length
    return {frameSize};
}

PayloadType SpeexCodec::getSupportedPlayloadType() const
{
    return PayloadType::SPEEX;
}

bool SpeexCodec::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    switch(audioConfig.sampleRate)
    {
        //narrow band
        case AudioConfiguration::SAMPLE_RATE_8000:
            speexEncoder = speex_encoder_init(&speex_nb_mode);
            speexDecoder = speex_decoder_init(&speex_nb_mode);
            break;
        //wide band
        case AudioConfiguration::SAMPLE_RATE_16000:
            speexEncoder = speex_encoder_init(&speex_wb_mode);
            speexDecoder = speex_decoder_init(&speex_wb_mode);
            break;
        //ultra-wide band
        case AudioConfiguration::SAMPLE_RATE_32000:
            speexEncoder = speex_encoder_init(&speex_uwb_mode);
            speexDecoder = speex_decoder_init(&speex_uwb_mode);
            break;
    }
    
    //XXX set quality ??! RFC 5574, section 5
    //XXX we could let the speex-codec generate comfort noise for us
    
    //set DTX
    bool useDTX = configMode->isCustomConfigurationSet(Parameters::ENABLE_DTX->longName, "Enable DTX");
    speex_encoder_ctl(speexEncoder, SPEEX_SET_DTX, &useDTX);
    
    return true;
}

unsigned int SpeexCodec::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    //reset bits before encoding frame
    speex_bits_reset(&encoderBits);
    
    if(speex_encode_int(speexEncoder, (spx_int16_t*)inputBuffer, &encoderBits) == 0)
        userData->isSilentPackage = true;
    
    //TODO RFC 5574 section 3.3: "The RTP payload MUST be padded to provide an integer number of octets as the payload length [...]"
    //is this automatically done?? if not, how to pad with bits
    return speex_bits_write(&encoderBits, (char*)inputBuffer, userData->maxBufferSize);
}

unsigned int SpeexCodec::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    const bool packageLost = userData->isSilentPackage;
    if(packageLost)
        speex_decode_int(speexDecoder, nullptr, (spx_int16_t*)outputBuffer);
    else
    {
        speex_bits_read_from(&decoderBits, (const char*)outputBuffer, outputBufferByteSize);
        speex_decode_int(speexDecoder, &decoderBits, (spx_int16_t*)outputBuffer);
    }
    
    //TODO what to return??
    return userData->maxBufferSize;
}

bool SpeexCodec::cleanUp()
{
    if(speexEncoder != nullptr)
    {
        speex_encoder_destroy(speexEncoder);
        speex_bits_destroy(&encoderBits);
    }
    if(speexDecoder != nullptr)
    {
        speex_decoder_destroy(speexDecoder);
        speex_bits_destroy(&decoderBits);
    }
    
    speexEncoder = nullptr;
    speexDecoder = nullptr;
    return true;
}

#endif