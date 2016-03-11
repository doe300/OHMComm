/* 
 * File:   ProcessoriLBC.cpp
 * Author: daniel
 * 
 * Created on March 2, 2016, 1:22 PM
 */
#ifdef ILBC_HEADER  //Only include of iLBC is linked

#include <iostream>

#include "codecs/ProcessoriLBC.h"

static constexpr ProcessorCapabilities iLBCCapabilities = {true, false, true, true, false, 0,
            1900   //304 Bit per 20 ms -> 38 Byte per 20 ms -> 1900 B/s
                    //400 Bit per 30 ms -> 50 Byte per 30 ms -> 1666 B/s
};

ProcessoriLBC::ProcessoriLBC(const std::string& name) : AudioProcessor(name, iLBCCapabilities), 
        iLBCEncoder(nullptr), iLBCDecoder(nullptr), frameLength(0)
{
    
}

ProcessoriLBC::~ProcessoriLBC()
{
    if(iLBCEncoder != nullptr)
        WebRtcIlbcfix_EncoderFree(iLBCEncoder);
    if(iLBCDecoder != nullptr)
        WebRtcIlbcfix_DecoderFree(iLBCDecoder);
}

unsigned int ProcessoriLBC::getSupportedAudioFormats() const
{
    //RFC 3952, section 2: "[...] compresses each basic frame(20 ms or 30 ms) of 8000 Hz, 16-bit sampled input speech, 
    //  into output frames with rate of 400 bits for 30 ms basic frame size and 304 bits for 20 ms basic frame size"
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

unsigned int ProcessoriLBC::getSupportedSampleRates() const
{
    //iLBC only supports 8kHz
    return AudioConfiguration::SAMPLE_RATE_8000;
}

const std::vector<int> ProcessoriLBC::getSupportedBufferSizes(unsigned int sampleRate) const
{
    //as of RFC 3952 section 2 use frame sizes of 20 and 30ms
    const int packageSize20ms = 20 * sampleRate / 1000;    //20 ms
    const int packageSize30ms = 30 * sampleRate / 1000;    //30 ms
    return {packageSize20ms, packageSize30ms};
}

PayloadType ProcessoriLBC::getSupportedPlayloadType() const
{
    return PayloadType::ILBC;
}

void ProcessoriLBC::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    if(WebRtcIlbcfix_EncoderCreate(&iLBCEncoder) != 0 || WebRtcIlbcfix_DecoderCreate(&iLBCDecoder) != 0)
    {
        throw ohmcomm::configuration_error("iLBC", "Error creating encoder and decoder!");
    }
    
    //if the buffer-size is 20ms of mono or stereo audio-data, use 20
    //else if the buffer-size contains 30ms of mono/stereo data, use 30
    const int bufferSize20ms = 20 * 8000 / 1000 * sizeof(int16_t);    //20 ms * sizeof(SINT16)
    const int bufferSize30ms = 30 * 8000 / 1000 * sizeof(int16_t);    //30 ms
    if(bufferSize % bufferSize20ms == 0)
        frameLength = 20;
    else if(bufferSize % bufferSize30ms == 0)
        frameLength = 30;
    else
    {
        throw ohmcomm::configuration_error("iLBC", std::string("Invalid buffer-size: ")+ std::to_string(bufferSize));
    }
    
    if(WebRtcIlbcfix_EncoderInit(iLBCEncoder, frameLength) != 0 || WebRtcIlbcfix_DecoderInit(iLBCDecoder, frameLength) != 0)
    {
        throw ohmcomm::configuration_error("iLBC", "Error initializing encoder or decoder!");
    }
    
    char buf[21];
    WebRtcIlbcfix_version(buf);
    std::cout << "iLBC: configured in version: " << buf << std::endl;
}

unsigned int ProcessoriLBC::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    const WebRtc_Word16 result = WebRtcIlbcfix_Encode(iLBCEncoder, (WebRtc_Word16*)inputBuffer, userData->nBufferFrames, (WebRtc_Word16*)inputBuffer);
    if(result < 0)
    {
        throw ohmcomm::playback_error("iLBC", "Error while encoding!");
    }
    return result;
}

unsigned int ProcessoriLBC::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    WebRtc_Word16 speechType;
    const bool packageLost = userData->isSilentPackage;
    WebRtc_Word16 result;
    if(packageLost)
    {
        result = WebRtcIlbcfix_DecodePlc(iLBCDecoder, (WebRtc_Word16*)outputBuffer, 1);
    }
    else
    {
        result = WebRtcIlbcfix_Decode(iLBCDecoder, (WebRtc_Word16*)outputBuffer, outputBufferByteSize, (WebRtc_Word16*)outputBuffer, &speechType);
        userData->nBufferFrames = result;
    }
    if(result < 0)
    {
        throw ohmcomm::playback_error("iLBC", "Error while decoding!");
    }
    return result;
}

bool ProcessoriLBC::cleanUp()
{
    if(iLBCEncoder != nullptr)
        WebRtcIlbcfix_EncoderFree(iLBCEncoder);
    if(iLBCDecoder != nullptr)
        WebRtcIlbcfix_DecoderFree(iLBCDecoder);
    iLBCEncoder = nullptr;
    iLBCDecoder = nullptr;
    
    return true;
}
#endif