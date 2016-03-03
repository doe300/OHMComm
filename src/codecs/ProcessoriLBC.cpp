/* 
 * File:   ProcessoriLBC.cpp
 * Author: daniel
 * 
 * Created on March 2, 2016, 1:22 PM
 */
#ifdef ILBC_HEADER  //Only include of iLBC is linked
//XXX wrong iLBC implementation, this one is from WebRTC, we need the source (from RFC)
//TODO is not yet tested due to incompatible sample-rates
#include <iostream>

#include "codecs/ProcessoriLBC.h"

ProcessoriLBC::ProcessoriLBC(const std::string& name) : AudioProcessor(name), iLBCEncoder(nullptr), iLBCDecoder(nullptr),
        frameLength(0), resampler("dummy")
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
    //iLBC only supports 8kHz, but we can resample the other sample-rates
    return AudioConfiguration::SAMPLE_RATE_8000|AudioConfiguration::SAMPLE_RATE_16000|AudioConfiguration::SAMPLE_RATE_24000|
            AudioConfiguration::SAMPLE_RATE_32000|AudioConfiguration::SAMPLE_RATE_48000|
            AudioConfiguration::SAMPLE_RATE_96000|AudioConfiguration::SAMPLE_RATE_192000;
}

const std::vector<int> ProcessoriLBC::getSupportedBufferSizes(unsigned int sampleRate) const
{
    //as of RFC 3952 section 2 use default frame sizes of 20 and 30ms
    const int packageSize20ms = 20 * 8000 / 1000;    //20 ms
    const int packageSize30ms = 30 * 8000 / 1000;    //30 ms
    return {packageSize20ms, packageSize30ms, BUFFER_SIZE_ANY};
}

PayloadType ProcessoriLBC::getSupportedPlayloadType() const
{
    return PayloadType::ILBC;
}

bool ProcessoriLBC::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    if(WebRtcIlbcfix_EncoderCreate(&iLBCEncoder) != 0 || WebRtcIlbcfix_DecoderCreate(&iLBCDecoder) != 0)
    {
        std::cerr << "iLBC: Error creating encoder and decoder!" << std::endl;
        return false;
    }
    
    //if the buffer-size is 20ms of mono or stereo audio-data, use 20
    //else if the buffer-size contains 30ms of mono/stereo data, use 30
    const int bufferSize20ms = 20 * 8000 / 1000 * sizeof(int16_t);    //20 ms * sizeof(SINT16)
    const int bufferSize30ms = 30 * 8000 / 1000 * sizeof(int16_t);    //30 ms
    if(bufferSize == bufferSize20ms || bufferSize == 2 * bufferSize20ms)
        frameLength = 20;
    else if(bufferSize == bufferSize30ms || bufferSize == 2 * bufferSize30ms)
        frameLength = 30;
    else
    {
        std::cerr << "iLBC: Invalid buffer-size: " << bufferSize << std::endl;
        return false;
    }
    
    if(WebRtcIlbcfix_EncoderInit(iLBCEncoder, frameLength) != 0 || WebRtcIlbcfix_DecoderInit(iLBCDecoder, frameLength) != 0)
    {
        std::cerr << "iLBC: Error initializing encoder or decoder!" << std::endl;
        return false;
    }
    
    return resampler.configure(audioConfig.audioFormatFlag, audioConfig.inputDeviceChannels, audioConfig.outputDeviceChannels, audioConfig.sampleRate/8000);
}

unsigned int ProcessoriLBC::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    resampler.processInputData(inputBuffer, inputBufferByteSize, userData);
    //XXX does in-place converting work??
    const WebRtc_Word16 result = WebRtcIlbcfix_Encode(iLBCEncoder, (WebRtc_Word16*)inputBuffer, userData->nBufferFrames, (WebRtc_Word16*)inputBuffer);
    if(result < 0)
    {
        std::cerr << "iLBC: Error while encoding!" << std::endl;
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
        result = WebRtcIlbcfix_NetEqPlc(iLBCDecoder, (WebRtc_Word16*)outputBuffer, userData->nBufferFrames);
    }
    else
    {
        //XXX does in-place converting work??
        result = WebRtcIlbcfix_Decode(iLBCDecoder, (WebRtc_Word16*)outputBuffer, outputBufferByteSize, (WebRtc_Word16*)outputBuffer, &speechType);
        result = resampler.processOutputData(outputBuffer, result * sizeof(WebRtc_Word16), userData);
    }
    if(result < 0)
    {
        std::cerr << "iLBC: Error while decoding!" << std::endl;
    }
    userData->nBufferFrames = result;
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
}
#endif