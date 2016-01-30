#include "RTAudioWrapper.h"
#include "Statistics.h"

// constructor
RtAudioWrapper::RtAudioWrapper() : AudioHandler(), bufferAudioOutput(nullptr), streamData(new StreamData())
{
}

// constructor
RtAudioWrapper::RtAudioWrapper(const AudioConfiguration &audioConfig) : RtAudioWrapper()
{
    this->setConfiguration(audioConfig);
}

RtAudioWrapper::~RtAudioWrapper()
{
    delete streamData;
    delete[] (char*)bufferAudioOutput;
}

// static callbackHelper (calls the callback of the object)
auto RtAudioWrapper::callbackHelper(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *rtAudioWrapperObject) -> int
{
    RtAudioWrapper *rtAudioWrapper = static_cast <RtAudioWrapper*> (rtAudioWrapperObject);
    return rtAudioWrapper->callback(outputBuffer, inputBuffer, nBufferFrames, streamTime, status, nullptr);
}

// callback of the object
auto RtAudioWrapper::callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *rtAudioWrapperObject) -> int
{
    if (status == RTAUDIO_INPUT_OVERFLOW)
    {
        // TODO: Create a log. Input data was discarded because of an overflow (data loss)
        std::cout << "Overflow\n";
    }

    if (status == RTAUDIO_OUTPUT_UNDERFLOW)
    {
        // TODO: Create a log. Output buffer ran low, produces a break in the output sound.
        std::cout << "Underflow\n";
    }

    this->streamData->nBufferFrames = nBufferFrames;
    //streamTime is the number of seconds since start of stream, so we convert to number of microseconds
    this->streamData->streamTime = lround(streamTime * 1000000);
    Statistics::setCounter(Statistics::TOTAL_ELAPSED_MILLISECONDS, streamTime * 1000);
    Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_RECORDED, inputBufferByteSize);
    Statistics::incrementCounter(Statistics::COUNTER_FRAMES_RECORDED, nBufferFrames);
    Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_OUTPUT, outputBufferByteSize);
    Statistics::incrementCounter(Statistics::COUNTER_FRAMES_OUTPUT, nBufferFrames);

    //in case the two buffer sizes max vary
    this->streamData->maxBufferSize = inputBufferByteSize;
    this->streamData->isSilentPackage = false;
    if (inputBuffer != nullptr)
        this->processAudioInput(inputBuffer, inputBufferByteSize, streamData);

    //in case the two buffer sizes max vary
    this->streamData->maxBufferSize = outputBufferByteSize;
    if (outputBuffer != nullptr)
        this->processAudioOutput(outputBuffer, outputBufferByteSize, streamData);

    return 0;
}

// Region: AudioWrapper methods
void RtAudioWrapper::startRecordingMode()
{
    if (this->flagPrepared)
    {
        this->rtaudio.openStream(nullptr, &input, audioConfiguration.audioFormatFlag, audioConfiguration.sampleRate, &audioConfiguration.framesPerPackage, &RtAudioWrapper::callbackHelper, this);
        this->rtaudio.startStream();
    }
    else
        std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
}

void RtAudioWrapper::startPlaybackMode()
{
    if (this->flagPrepared)
    {
            this->rtaudio.openStream(&output, nullptr, audioConfiguration.audioFormatFlag, audioConfiguration.sampleRate, &audioConfiguration.framesPerPackage, &RtAudioWrapper::callbackHelper, this);
            this->rtaudio.startStream();
    }
    else
            std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
}

void RtAudioWrapper::startDuplexMode()
{
    if (this->flagPrepared)
    {
        this->rtaudio.openStream(&output, &input, audioConfiguration.audioFormatFlag, audioConfiguration.sampleRate, &audioConfiguration.framesPerPackage, &RtAudioWrapper::callbackHelper, this);
        this->rtaudio.startStream();
    }
    else
        std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
}


void RtAudioWrapper::setConfiguration(const AudioConfiguration &audioConfig)
{
    this->audioConfiguration = audioConfig;
    this->flagAudioConfigSet = true;
}

void RtAudioWrapper::suspend()
{
    if (this->rtaudio.isStreamRunning())
    {
        this->rtaudio.stopStream();
    }	
}

void RtAudioWrapper::stop()
{
    this->suspend();
    //FIXME ALSA-API throws "duplicate free or corruption" in closeStream()
    this->rtaudio.closeStream();
    this->cleanUpAudioProcessors();
}

void RtAudioWrapper::resume()
{
    if (this->rtaudio.isStreamOpen() && this->rtaudio.isStreamRunning() == false)
        this->rtaudio.startStream();
}

void RtAudioWrapper::reset()
{
    this->stop();
    this->audioConfiguration = { 0 };
    this->flagAudioConfigSet = false;
    this->flagPrepared = false;
}


// Region: private functions
auto RtAudioWrapper::initRtAudioStreamParameters() -> bool
{
    // calculate the input- and outputbuffer sizes
    this->outputBufferByteSize = audioConfiguration.framesPerPackage * audioConfiguration.outputDeviceChannels * getAudioFormatByteSize(audioConfiguration.audioFormatFlag);
    this->inputBufferByteSize = audioConfiguration.framesPerPackage * audioConfiguration.inputDeviceChannels * getAudioFormatByteSize(audioConfiguration.audioFormatFlag);

    /* internal buffer for playback data */
    this->bufferAudioOutput = new char[this->outputBufferByteSize];

    /* Prepare the StreamParameters */
    this->input.deviceId = audioConfiguration.inputDeviceID;
    this->output.deviceId = audioConfiguration.outputDeviceID;
    this->input.nChannels = audioConfiguration.inputDeviceChannels;
    this->output.nChannels = audioConfiguration.outputDeviceChannels;

    return true;
}

void RtAudioWrapper::setDefaultAudioConfig()
{
    AudioConfiguration audioConfig{};
    audioConfig.inputDeviceID = this->rtaudio.getDefaultInputDevice();
    audioConfig.outputDeviceID = this->rtaudio.getDefaultOutputDevice();

    //input device
//    RtAudio::DeviceInfo inputDeviceInfo = this->rtaudio.getDeviceInfo(audioConfig.inputDeviceID);
//    RtAudio::DeviceInfo outputDeviceInfo = this->rtaudio.getDeviceInfo(audioConfig.outputDeviceID);

    audioConfig.inputDeviceChannels = 2;
    audioConfig.outputDeviceChannels = 2;
    //RtAudioFormat rtaudioFormat = autoSelectAudioFormat(outputDeviceInfo.nativeFormats);
    //audioConfig.audioFormat = getAudioFormatByteSize(rtaudioFormat);
    //audioFormat sampleRate and bufferFrames are overridden by queryProcessorSupport()
    audioConfig.audioFormatFlag = 0;
    audioConfig.sampleRate = 0;
    audioConfig.framesPerPackage = 0;

    this->setConfiguration(audioConfig);
}

auto RtAudioWrapper::getAudioFormatByteSize(RtAudioFormat rtaudioFormat) -> int
{
    switch (rtaudioFormat)
    {
        case(RTAUDIO_SINT8): return 1;
        case(RTAUDIO_SINT16) : return 2;
        case(RTAUDIO_SINT24) : return 3;
        case(RTAUDIO_SINT32) : return 4;
        case(RTAUDIO_FLOAT32) : return 4;
        case(RTAUDIO_FLOAT64) : return 8;
    }
    return 0;
}

auto RtAudioWrapper::getOutputFrameSize() -> int
{
    if (this->flagAudioConfigSet)
        return getAudioFormatByteSize(this->audioConfiguration.audioFormatFlag) * this->audioConfiguration.outputDeviceChannels;
    return 0;
}

auto RtAudioWrapper::getInputFrameSize() -> int
{
	if (this->flagAudioConfigSet)
		return getAudioFormatByteSize(this->audioConfiguration.audioFormatFlag) * this->audioConfiguration.inputDeviceChannels;
	return 0;
}

auto RtAudioWrapper::autoSelectAudioFormat(RtAudioFormat supportedFormats) -> RtAudioFormat
{
    if ((supportedFormats & RTAUDIO_FLOAT64) == RTAUDIO_FLOAT64)
    {
        return RTAUDIO_FLOAT64;
    }
    if ((supportedFormats & RTAUDIO_FLOAT32) == RTAUDIO_FLOAT32)
    {
        return RTAUDIO_FLOAT32; 
    }
    if ((supportedFormats & RTAUDIO_SINT32) == RTAUDIO_SINT32)
    {
        return RTAUDIO_SINT32;
    }
    if ((supportedFormats & RTAUDIO_SINT24) == RTAUDIO_SINT24)
    {
        return RTAUDIO_SINT24;
    }
    if ((supportedFormats & RTAUDIO_SINT16) == RTAUDIO_SINT16)
    {
        return RTAUDIO_SINT16;
    }
    //fall back to worst quality
    return RTAUDIO_SINT8;
}

unsigned int RtAudioWrapper::autoSelectSampleRate(unsigned int supportedRatesFlag)
{
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_192000) == AudioConfiguration::SAMPLE_RATE_192000)
    {
        return 192000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_96000) == AudioConfiguration::SAMPLE_RATE_96000)
    {
        return 96000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_48000) == AudioConfiguration::SAMPLE_RATE_48000)
    {
        return 48000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_44100) == AudioConfiguration::SAMPLE_RATE_44100)
    {
        return 44100;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_32000) == AudioConfiguration::SAMPLE_RATE_32000)
    {
        return 32000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_24000) == AudioConfiguration::SAMPLE_RATE_24000)
    {
        return 24000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_16000) == AudioConfiguration::SAMPLE_RATE_16000)
    {
        return 16000;
    }
    if((supportedRatesFlag & AudioConfiguration::SAMPLE_RATE_12000) == AudioConfiguration::SAMPLE_RATE_12000)
    {
        return 12000;
    }
    //fall back to worst sample-rate
    return 8000;
}


bool RtAudioWrapper::prepare(const std::shared_ptr<ConfigurationMode> configMode)
{
    /* If there is no config set, then load the default */
    if (this->flagAudioConfigSet == false)
        this->setDefaultAudioConfig();
    
    //checks if there is a configuration all processors support
    if(!queryProcessorSupport())
    {
        std::cerr << "AudioProcessors could not agree on configuration!" << std::endl;
        return false;
    }
    
    bool resultA = this->initRtAudioStreamParameters();
    bool resultB = this->configureAudioProcessors(configMode);

    if (resultA && resultB) {
        this->flagPrepared = true;
        return true;
    }

    return false;
}

auto RtAudioWrapper::getBufferSize() -> unsigned int
{
    return outputBufferByteSize;
}

bool RtAudioWrapper::queryProcessorSupport()
{
    //preset supported audio-formats with device-supported formats
    //although there is a value for native audio-formats in the device-info, RtAudio documentation says:
    // "However, RtAudio will automatically provide format conversion if a particular format is not natively supported."
    unsigned int supportedFormats = AudioConfiguration::AUDIO_FORMAT_ALL;
    if(audioConfiguration.forceAudioFormatFlag != 0)
    {
        //force the specific audio-format
        supportedFormats = audioConfiguration.forceAudioFormatFlag;
    }
    //preset supported sample-rates with device-supported rates
    unsigned int supportedSampleRates = mapDeviceSampleRates(this->rtaudio.getDeviceInfo(audioConfiguration.inputDeviceID).sampleRates);
    if(audioConfiguration.forceSampleRate != 0)
    {
        //force the specific sample-rate
        supportedSampleRates = mapDeviceSampleRates({audioConfiguration.forceSampleRate});
    }
    for (unsigned int i = 0; i < audioProcessors.size(); i++)
    {
        // a & b return all bits set in a AND b -> all flags supported by both
        supportedFormats = supportedFormats & audioProcessors.at(i)->getSupportedAudioFormats();
        supportedSampleRates = supportedSampleRates &audioProcessors.at(i)->getSupportedSampleRates();
    }
    if(supportedFormats == 0)
    {
        //there is no format supported by all processors
        std::cerr << "Could not find a single audio-format supported by all processors!" << std::endl;
        return false;
    }
    if(supportedSampleRates == 0)
    {
        //there is no sample-rate supported by all processors
        std::cerr << "Could not find a single sample-rate supported by all processors!" << std::endl;
        return false;
    }
    audioConfiguration.audioFormatFlag = autoSelectAudioFormat(supportedFormats);
    audioConfiguration.sampleRate = autoSelectSampleRate(supportedSampleRates);
    
    //find common supported buffer-size, defaults to 512
    int supportedBufferSize = findOptimalBufferSize(512);
    if(supportedBufferSize == 0)
    {
        std::cerr << "Could not find a single buffer-size supported by all processors!" << std::endl;
        return false;
    }
    audioConfiguration.framesPerPackage = supportedBufferSize;
    
    std::cout << "Using audio-format: " << AudioConfiguration::getAudioFormatDescription(audioConfiguration.audioFormatFlag, false) << std::endl;
    std::cout << "Using a sample-rate of " << audioConfiguration.sampleRate << " Hz" << std::endl;
    std::cout << "Using a buffer-size of " << supportedBufferSize << " samples (" << (supportedBufferSize * 1000 / audioConfiguration.sampleRate) << " ms)" << std::endl;
    
    return true;
}

unsigned int RtAudioWrapper::mapDeviceSampleRates(std::vector<unsigned int> sampleRates)
{
    unsigned int sampleRate;
    unsigned int sampleRatesFlags = 0;
    for (unsigned int i = 0; i < sampleRates.size(); i++)
    {
        sampleRate = sampleRates.at(i);
        if(sampleRate == 8000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_8000;
        else if(sampleRate == 12000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_12000;
        else if(sampleRate == 16000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_16000;
        else if(sampleRate == 24000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_24000;
        else if(sampleRate == 32000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_32000;
        else if(sampleRate == 44100)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_44100;
        else if(sampleRate == 48000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_48000;
        else if(sampleRate == 96000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_96000;
        else if(sampleRate == 192000)
            sampleRatesFlags |= AudioConfiguration::SAMPLE_RATE_192000;
        else
            std::cout << "Unrecognized sample-rate: " << sampleRate << std::endl;
    }
    
    return sampleRatesFlags;
}

unsigned int RtAudioWrapper::findOptimalBufferSize(unsigned int defaultBufferSize)
{
    //get all supported buffer-sizes
    std::vector<std::vector<int>> processorBufferSizes(audioProcessors.size());
    for (unsigned int i = 0; i < audioProcessors.size(); i++)
    {
        processorBufferSizes[i] = audioProcessors[i]->getSupportedBufferSizes(audioConfiguration.sampleRate);
    }
    //number of processors supporting all buffer-sizes
    std::vector<bool> processorSupportAll(processorBufferSizes.size(), false);
    //iterate through all priorities and check if the value is supported by other processors
    for(unsigned int sizeIndex = 0; sizeIndex < 32; sizeIndex ++)
    {
        for(unsigned int procIndex = 0; procIndex < processorBufferSizes.size(); procIndex ++)
        {
            //processor has no size-entries left
            if(processorBufferSizes[procIndex].size() <= sizeIndex)
            {
                continue;
            }
            if(processorBufferSizes[procIndex][sizeIndex] == BUFFER_SIZE_ANY)
            {
                processorSupportAll[procIndex] = true;
                continue;
            }
            //"suggest" buffer-size
            int suggestedBufferSize = processorBufferSizes[procIndex][sizeIndex];
            //check "suggested" buffer-size against all other processors
            bool suggestionAccepted = true;
            for(unsigned int checkProcIndex = 0; checkProcIndex < processorBufferSizes.size(); checkProcIndex ++)
            {
                bool singleProcessorAccepted = false;
                //we already checked all sizes below sizeIndex
                for(unsigned int checkSizeIndex = sizeIndex; checkSizeIndex < processorBufferSizes[checkProcIndex].size(); checkSizeIndex ++)
                {
                    if(processorBufferSizes[checkProcIndex][checkSizeIndex] == suggestedBufferSize || processorBufferSizes[checkProcIndex][checkSizeIndex] == BUFFER_SIZE_ANY)
                    {
                        singleProcessorAccepted = true;
                        //break;
                    }
                }
                if(!singleProcessorAccepted)
                {
                    suggestionAccepted = false;
                    //break;
                }
            }
            if(suggestionAccepted)
            {
                return suggestedBufferSize;
            }
        }
    }
    //if all processors only have BUFFER_SIZE_ALL, return default-value
    bool supportAll = true;
    for(unsigned int procIndex = 0; procIndex < processorSupportAll.size(); procIndex ++)
    {
        if(!processorSupportAll[procIndex])
        {
            supportAll = false;
            break;
        }
    }
    if(supportAll)
    {
        return defaultBufferSize;
    }
    
    return 0;
}
