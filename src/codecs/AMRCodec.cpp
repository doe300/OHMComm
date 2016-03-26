/* 
 * File:   AMRCodec.cpp
 * Author: daniel
 * 
 * Created on March 2, 2016, 1:39 PM
 */

#ifdef AMR_ENCODER_HEADER //Only compile if AMR is linked

#include "codecs/AMRCodec.h"
#include "Parameters.h"

using namespace ohmcomm::codecs;

static constexpr ohmcomm::ProcessorCapabilities amrCapabilities = {true, false, true, true, false, 0, 1525 /* highest mode says 12.2kbps */};

AMRCodec::AMRCodec(const std::string& name) : AudioProcessor(name, amrCapabilities), amrEncoder(nullptr), amrDecoder(nullptr)
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
    return {defaultPackageSize};
}

ohmcomm::PayloadType AMRCodec::getSupportedPlayloadType() const
{
    return PayloadType::AMR_NB;
}

void AMRCodec::configure(const ohmcomm::AudioConfiguration& audioConfig, const std::shared_ptr<ohmcomm::ConfigurationMode> configMode, const uint16_t bufferSize, const ProcessorCapabilities& chainCapabilities)
{
    const bool useDTX = configMode->isCustomConfigurationSet(Parameters::ENABLE_DTX->longName, "Enable DTX");
    amrEncoder = Encoder_Interface_init(useDTX);
    amrDecoder = Decoder_Interface_init();
}

unsigned int AMRCodec::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, ohmcomm::StreamData* userData)
{
    const bool isSilence = userData->isSilentPackage;
    return Encoder_Interface_Encode(amrEncoder, isSilence ? Mode::MRDTX : Mode::MR122, (const short*)inputBuffer, (unsigned char*)inputBuffer, true);
}

unsigned int AMRCodec::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, ohmcomm::StreamData* userData)
{
    const bool dtx = userData->isSilentPackage;
    Decoder_Interface_Decode(amrDecoder, (unsigned char*)outputBuffer, (short*)outputBuffer, dtx);
    userData->nBufferFrames = 160;
    return 160 * sizeof(int16_t);
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
    
    return true;
}
#endif