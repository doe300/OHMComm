/* 
 * File:   AMRCodec.cpp
 * Author: daniel
 * 
 * Created on March 2, 2016, 1:39 PM
 */

#ifdef AMR_ENCODER_HEADER //Only compile if AMR is linked

#include "codecs/AMRCodec.h"
#include "Parameters.h"

//FIXME doesn't work yet

AMRCodec::AMRCodec(const std::string& name) : AudioProcessor(name), amrEncoder(nullptr), amrDecoder(nullptr)
{
}

AMRCodec::~AMRCodec()
{
}

unsigned int AMRCodec::getSupportedAudioFormats() const
{
    //only supports 16bit signed integer as input
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

unsigned int AMRCodec::getSupportedSampleRates() const
{
    //RFC 4867 section 4.1: "For AMR, the sampling frequency is 8 kHz, corresponding to 160 encoded speech samples per frame from each channel"
    return AudioConfiguration::SAMPLE_RATE_8000;
}

const std::vector<int> AMRCodec::getSupportedBufferSizes(unsigned int sampleRate) const
{
    //RFC 4867 section 4.1: "The duration of one speech frame-block is 20 ms [...]"
    const int defaultPackageSize = 20 * sampleRate / 1000;    //20 ms
    return {defaultPackageSize, BUFFER_SIZE_ANY};
}

PayloadType AMRCodec::getSupportedPlayloadType() const
{
    return PayloadType::AMR_NB;
}

bool AMRCodec::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    const bool useDTX = configMode->isCustomConfigurationSet(Parameters::ENABLE_DTX->longName, "Enable DTX");
    amrEncoder = Encoder_Interface_init(useDTX);
    amrDecoder = Decoder_Interface_init();
    //all configured
    return true;
}

unsigned int AMRCodec::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    const bool isSilence = userData->isSilentPackage;
    //XXX does in-place converting work??
    return Encoder_Interface_Encode(amrEncoder, isSilence ? Mode::MRDTX : Mode::MR122, (const short*)inputBuffer, (unsigned char*)inputBuffer, true);
}

unsigned int AMRCodec::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    const bool dtx = userData->isSilentPackage;
    //XXX does this work??
    Decoder_Interface_Decode(amrDecoder, (unsigned char*)outputBuffer, (short*)outputBuffer, dtx);
    //XXX need to set number of samples
    //TODO what to return here??
    return userData->maxBufferSize;
}

bool AMRCodec::cleanUp()
{
    if(amrEncoder != nullptr)
    {
        Encoder_Interface_exit(amrEncoder);
        amrEncoder = nullptr;
    }
    if(amrDecoder != nullptr)
    {
        Decoder_Interface_exit(amrDecoder);
        amrDecoder = nullptr;
    }
}
#endif