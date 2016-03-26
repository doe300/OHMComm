#ifdef OPUS_HEADER //Only compile, if opus is linked
#include "codecs/OpusCodec.h"
#include "Parameters.h"

using namespace ohmcomm::codecs;

static constexpr ohmcomm::ProcessorCapabilities opusCapabilities = {true, true, true, true, false, 0, 0};


OpusCodec::OpusCodec(const std::string name) : 
    AudioProcessor(name, opusCapabilities), OpusEncoderObject(nullptr), OpusDecoderObject(nullptr), useFEC(false)
{
}

OpusCodec::~OpusCodec()
{
    if(OpusEncoderObject != nullptr)
        opus_encoder_destroy(OpusEncoderObject);
    if(OpusDecoderObject != nullptr)
        opus_decoder_destroy(OpusDecoderObject);
}

unsigned int OpusCodec::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_SINT16|AudioConfiguration::AUDIO_FORMAT_FLOAT32;
}

unsigned int OpusCodec::getSupportedSampleRates() const
{
    unsigned int SupportedSampleRates = AudioConfiguration::SAMPLE_RATE_8000|AudioConfiguration::SAMPLE_RATE_12000|AudioConfiguration::SAMPLE_RATE_16000|AudioConfiguration::SAMPLE_RATE_24000|AudioConfiguration::SAMPLE_RATE_48000;
    return SupportedSampleRates;
}

const std::vector<int> OpusCodec::getSupportedBufferSizes(unsigned int sampleRate) const
{
    if (sampleRate == 8000)
    {
        return std::vector<int>({ 480, 320, 160, 80, 40, 20 });
    }
    else if (sampleRate == 12000)
    {
        return std::vector<int>({ 720, 480, 240, 120, 60, 30 });
    }
    else if (sampleRate == 16000)
    {
        return std::vector<int>({ 960, 640, 320, 160, 80, 40 });
    }
    else if (sampleRate == 24000)
    {
        return std::vector<int>({ 1440, 960, 480, 240, 120, 60 });
    }
    else if (sampleRate == 48000)
    {
        return std::vector<int>({ 960, 1920, 2880, 480, 240, 120 });
    }
    else
    {
        std::cerr << "Opus: No supported buffer size found!" << std::endl;
        return std::vector<int>({ });
    }
}

ohmcomm::PayloadType OpusCodec::getSupportedPlayloadType() const
{
    return PayloadType::OPUS;
}

void OpusCodec::configure(const ohmcomm::AudioConfiguration& audioConfig, const std::shared_ptr<ohmcomm::ConfigurationMode> configMode, const uint16_t bufferSize, const ProcessorCapabilities& chainCapabilities)
{
    outputDeviceChannels = audioConfig.outputDeviceChannels;
    rtaudioFormat = audioConfig.audioFormatFlag;

    int errorCode = OPUS_OK;
    OpusEncoderObject = opus_encoder_create(audioConfig.sampleRate, audioConfig.inputDeviceChannels, OPUS_APPLICATION_VOIP, &errorCode);
    OpusDecoderObject = opus_decoder_create(audioConfig.sampleRate, audioConfig.outputDeviceChannels, &errorCode);
    
    if (errorCode != OPUS_OK)
    {
        throw ohmcomm::configuration_error("Opus", opus_strerror(errorCode));
    }
    
    //enable DTX for Opus, if enabled generally
    opus_encoder_ctl(OpusEncoderObject, OPUS_SET_DTX(configMode->isCustomConfigurationSet(Parameters::ENABLE_DTX->longName, "Enable DTX")));
    
    //enable FEC for opus
    //XXX FEC doesn't work yet, is not added to package??
    if(configMode->isCustomConfigurationSet(Parameters::ENABLE_FEC->longName, "Enable FEC"))
    {
        //useFEC = true;
        opus_encoder_ctl(OpusEncoderObject, OPUS_SET_INBAND_FEC(1));
    }
    std::cout << "Opus: configured using version: " << opus_get_version_string() << std::endl;
    
    //XXX rewrite, so sint16/float32 is only checked once and not on every run
}

unsigned int OpusCodec::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, ohmcomm::StreamData *userData)
{
    int lengthEncodedPacketInBytes = 0;
    if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_SINT16)
    {
        lengthEncodedPacketInBytes = opus_encode(OpusEncoderObject, (opus_int16 *)inputBuffer, userData->nBufferFrames, (unsigned char *)inputBuffer, userData->maxBufferSize);
    }
    else if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_FLOAT32)
    {
        lengthEncodedPacketInBytes = opus_encode_float(OpusEncoderObject, (const float *)inputBuffer, userData->nBufferFrames, (unsigned char *)inputBuffer, userData->maxBufferSize);
    }
    else
    {
        throw ohmcomm::playback_error("Opus", "No matching encoder found!");
    }
    //if output length is 1, DTX is applied
    if(lengthEncodedPacketInBytes == 1)
    {
        userData->isSilentPackage = true;
    }
    else if(lengthEncodedPacketInBytes < 0)
    {
        throw ohmcomm::playback_error("Opus", opus_strerror(lengthEncodedPacketInBytes));
    }
    return lengthEncodedPacketInBytes;
}

unsigned int OpusCodec::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, ohmcomm::StreamData *userData)
{
    const bool packageLost = userData->isSilentPackage;
    unsigned int numberOfDecodedSamples = 0;
    if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_SINT16)
    {
        if(packageLost) //to trigger PLC (package loss concealment), set data to nullptr
            numberOfDecodedSamples = opus_decode(OpusDecoderObject, nullptr, outputBufferByteSize, (opus_int16 *)outputBuffer, userData->nBufferFrames, useFEC);
        else
            numberOfDecodedSamples = opus_decode(OpusDecoderObject, (unsigned char *)outputBuffer, outputBufferByteSize, (opus_int16 *)outputBuffer, userData->nBufferFrames, useFEC);
        userData->nBufferFrames = numberOfDecodedSamples;
        const unsigned int outputBufferInBytes = (numberOfDecodedSamples * sizeof(opus_int16) * outputDeviceChannels);
        return outputBufferInBytes;
    }
    else if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_FLOAT32)
    {
        if(packageLost) //to trigger PLC (package loss concealment), set data to nullptr
            numberOfDecodedSamples = opus_decode_float(OpusDecoderObject, nullptr, outputBufferByteSize, (float *)outputBuffer, userData->nBufferFrames, useFEC);
        else
            numberOfDecodedSamples = opus_decode_float(OpusDecoderObject, (const unsigned char *)outputBuffer, outputBufferByteSize, (float *)outputBuffer, userData->nBufferFrames, useFEC);
        userData->nBufferFrames = numberOfDecodedSamples;
        const unsigned int outputBufferInBytes = (numberOfDecodedSamples * sizeof(float) * outputDeviceChannels);
        return outputBufferInBytes;
    }
    else
    {
        throw ohmcomm::playback_error("Opus", "No matching decoder found!");
    }
}

#endif