#include "RTAudioWrapper.h"

// constructor
RtAudioWrapper::RtAudioWrapper()
{
    this->audioConfiguration = { 0 };
    streamData = new StreamData();

    #ifdef _WIN32
    semaphore_waitForMainThread = CreateSemaphore(nullptr, 1, 1, "semaphore_waitForMainThread");
    semaphore_waitForWorkerThread = CreateSemaphore(nullptr, 0, 1, "semaphore_waitForWorkerThread");
    #else
            /* TODO, two linux semaphores needed */
    #endif
}

// constructor
RtAudioWrapper::RtAudioWrapper(const AudioConfiguration &audioConfig) : RtAudioWrapper()
{
    this->setConfiguration(audioConfig);
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

    // relevant if playData(..) is invoked
    this->playbackFileData(outputBuffer);

    this->streamData->nBufferFrames = nBufferFrames;
    //streamTime is the number of seconds since start of stream, so we convert to number of microseconds
    this->streamData->streamTime = lround(streamTime * 1000000);

    //in case the two buffer sizes max vary
    this->streamData->maxBufferSize = inputBufferByteSize;
    if (inputBuffer != nullptr)
        this->processAudioInput(inputBuffer, inputBufferByteSize, streamData);

    //in case the two buffer sizes max vary
    this->streamData->maxBufferSize = outputBufferByteSize;
    if (outputBuffer != nullptr)
        this->processAudioOutput(outputBuffer, outputBufferByteSize, streamData);

    return 0;
}

void RtAudioWrapper::playbackFileData(void *outputBuffer)
{
    if (this->BufferAudioOutHasData)
    {
        // critical section starts
        #ifdef _WIN32
        WaitForSingleObject(semaphore_waitForMainThread, INFINITE);
        #endif

        char *buffer = (char *)outputBuffer;
        memcpy(buffer, bufferAudioOutput, outputBufferByteSize);
        this->BufferAudioOutHasData = false;

        #ifdef _WIN32
        ReleaseSemaphore(semaphore_waitForMainThread, 1, nullptr);

        // critical section ends

        // inform that the data has been processed
        ReleaseSemaphore(semaphore_waitForWorkerThread, 1, nullptr);
        #endif
    }
    else
    {
        // If there is no data to be played then clean the buffer (otherwise it will cause a wired 
        // sound, since the buffer is used for playback over and over again with the same playback data)

        // TODO: getOutputData() and copy to outputbuffer;
    }
}


// Region: AudioWrapper methods
void RtAudioWrapper::startRecordingMode()
{
    if (this->flagPrepared)
    {
        this->rtaudio.openStream(nullptr, &input, audioConfiguration.audioFormat, audioConfiguration.sampleRate, &audioConfiguration.bufferFrames, &RtAudioWrapper::callbackHelper, this);
        this->rtaudio.startStream();
    }
    else
        std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
}

void RtAudioWrapper::startPlaybackMode()
{
    if (this->flagPrepared)
    {
            this->rtaudio.openStream(&output, nullptr, audioConfiguration.audioFormat, audioConfiguration.sampleRate, &audioConfiguration.bufferFrames, &RtAudioWrapper::callbackHelper, this);
            this->rtaudio.startStream();
    }
    else
            std::cout << "Did you forget to call AudioHandler::prepare()?" << std::endl;
}

void RtAudioWrapper::startDuplexMode()
{
    if (this->flagPrepared)
    {
        this->rtaudio.openStream(&output, &input, audioConfiguration.audioFormat, audioConfiguration.sampleRate, &audioConfiguration.bufferFrames, &RtAudioWrapper::callbackHelper, this);
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
        #ifdef _WIN32
        CloseHandle(semaphore_waitForMainThread);
        CloseHandle(semaphore_waitForWorkerThread);
        #endif
        this->rtaudio.stopStream();
    }	
}

void RtAudioWrapper::stop()
{
    this->suspend();
    this->rtaudio.closeStream();
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

void RtAudioWrapper::playData(void *playbackData, unsigned int size)
{
    char *buffer = (char *)playbackData;
    int dataLoops = (int) ceil((double)size / outputBufferByteSize);

    for (int i = 0; i < dataLoops; i++)
    {
        // critical section starts
        #ifdef _WIN32
        WaitForSingleObject(semaphore_waitForMainThread, INFINITE);
        #endif
        this->BufferAudioOutHasData = true;

        /* In the last loop, the size of the data may not be as big as bufferByteSize
        * In this special case, calculate the size of the remaining data
        */
        if (i + 1 == dataLoops && i != 0)
        {
            // calculate the size of  the remaining data
            int dataSizeLastLoop = (int) ceil(size % outputBufferByteSize);

            // first clean the buffer
            //memcpy(bufferAudioOut, 0x0, bufferByteSize);

            // fill the buffer with the remaining data
            memcpy(bufferAudioOutput, (buffer + i * dataSizeLastLoop), dataSizeLastLoop);
        }
        else
            memcpy(bufferAudioOutput, (buffer + i * outputBufferByteSize), outputBufferByteSize);

        #ifdef _WIN32
        ReleaseSemaphore(semaphore_waitForMainThread, 1, nullptr);
        // critical section ends

        // wait until the data is processed
        WaitForSingleObject(semaphore_waitForWorkerThread, INFINITE);
        #endif
    }
}


// Region: private functions
auto RtAudioWrapper::initRtAudioStreamParameters() -> bool
{
    // calculate the input- and outputbuffer sizes
    this->outputBufferByteSize = audioConfiguration.bufferFrames * audioConfiguration.outputDeviceChannels * getAudioFormatByteSize(audioConfiguration.audioFormat);
    this->inputBufferByteSize = audioConfiguration.bufferFrames * audioConfiguration.inputDeviceChannels * getAudioFormatByteSize(audioConfiguration.audioFormat);

    /* internal buffer for playback data */
    this->bufferAudioOutput = new char[this->outputBufferByteSize];

    /* Prepare the StreamParameters */
    this->input.deviceId = audioConfiguration.inputDeviceID;
    this->output.deviceId = audioConfiguration.outputDeviceID;
    this->input.nChannels = audioConfiguration.inputDeviceChannels;
    this->output.nChannels = audioConfiguration.outputDeviceChannels;
    this->input.firstChannel = audioConfiguration.inputDeviceFirstChannel;
    this->output.firstChannel = audioConfiguration.outputDeviceFirstChannel;

    return true;
}

void RtAudioWrapper::setDefaultAudioConfig()
{
    AudioConfiguration audioConfig = { 0 };
    audioConfig.inputDeviceID = this->rtaudio.getDefaultInputDevice();
    audioConfig.outputDeviceID = this->rtaudio.getDefaultOutputDevice();

    //input device
    RtAudio::DeviceInfo inputDeviceInfo = this->rtaudio.getDeviceInfo(audioConfig.inputDeviceID);
    RtAudio::DeviceInfo outputDeviceInfo = this->rtaudio.getDeviceInfo(audioConfig.outputDeviceID);

    audioConfig.inputDeviceChannels = 2;
    audioConfig.outputDeviceChannels = 2;
    //RtAudioFormat rtaudioFormat = autoSelectAudioFormat(outputDeviceInfo.nativeFormats);
	//audioConfig.audioFormat = getAudioFormatByteSize(rtaudioFormat);
	audioConfig.audioFormat = RTAUDIO_SINT16;
    audioConfig.sampleRate = 48000;
	audioConfig.bufferFrames = 960;

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
        return getAudioFormatByteSize(this->audioConfiguration.audioFormat) * this->audioConfiguration.outputDeviceChannels;
    return 0;
}

auto RtAudioWrapper::getInputFrameSize() -> int
{
	if (this->flagAudioConfigSet)
		return getAudioFormatByteSize(this->audioConfiguration.audioFormat) * this->audioConfiguration.inputDeviceChannels;
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


bool RtAudioWrapper::prepare()
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
    bool resultB = this->configureAudioProcessors();

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
    //preset supported sample-rates with device-supported rates
    unsigned int supportedSampleRates = mapDeviceSampleRates(this->rtaudio.getDeviceInfo(audioConfiguration.inputDeviceID).sampleRates);
    for (unsigned int i = 0; i < audioProcessors.size(); i++)
    {
        // a & b return all bits set in a AND b -> all flags supported by both
        supportedFormats = supportedFormats & audioProcessors.at(i)->getSupportedAudioFormats();
        supportedSampleRates = supportedSampleRates &audioProcessors.at(i)->getSupportedSampleRates();
    }
    if(supportedFormats == 0)
    {
        //there is no format supported by all processors
        std::cout << "Could not find a single audio-format supported by all processors!" << std::endl;
        return false;
    }
    if(supportedSampleRates == 0)
    {
        //there is no sample-rate supported by all processors
        std::cout << "Could not find a single sample-rate supported by all processors!" << std::endl;
        return false;
    }
    audioConfiguration.audioFormat = autoSelectAudioFormat(supportedFormats);
    audioConfiguration.sampleRate = autoSelectSampleRate(supportedSampleRates);
    
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
