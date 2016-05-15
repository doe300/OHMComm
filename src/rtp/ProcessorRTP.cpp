#include "Logger.h"
#include "rtp/ProcessorRTP.h"
#include "Statistics.h"
#include "Parameters.h"
#include "network/UDPWrapper.h"
#include "rtp/RTPBuffer.h"

using namespace ohmcomm::rtp;

ProcessorRTP::ProcessorRTP(const std::string name, const ohmcomm::NetworkConfiguration& networkConfig, const ohmcomm::PayloadType payloadType) : 
    AudioProcessor(name), network(new ohmcomm::network::UDPWrapper(networkConfig)), buffers(128, 200, 1), ourselves(ParticipantDatabase::self()), lastPackageWasSilent(false),
        totalSilenceDelayPackages(0), currentSilenceDelayPackages(0)
        //XXX make jitter-settings configurable (or at least use better values)
{
    ourselves.payloadType = payloadType;
}

void ProcessorRTP::configure(const ohmcomm::AudioConfiguration& audioConfig, const std::shared_ptr<ohmcomm::ConfigurationMode> configMode, const uint16_t bufferSize, const ohmcomm::ProcessorCapabilities& chainCapabilities)
{
    //check whether to enable DTX at all
    //check if any processor with VAD capabilities exists before asking for configuration-value
    if(chainCapabilities.canDetectSilence)
    {
        isDTXEnabled = configMode->isCustomConfigurationSet(Parameters::ENABLE_DTX->longName, "Enable DTX");
        if(isDTXEnabled)
        {
            ohmcomm::info("RTP") << "Using DTX after " << ProcessorRTP::SILENCE_DELAY << " ms of silence." << ohmcomm::endl;
            //calculate the number of packages to fill the specified delay
            const double timeOfPackage = audioConfig.framesPerPackage / (double)audioConfig.sampleRate;
            totalSilenceDelayPackages = (SILENCE_DELAY /1000.0) / timeOfPackage;
        }
    }
    rtpListener.reset(new RTPListener(network, buffers, bufferSize));
    rtcpHandler.reset(new RTCPHandler(configMode->getRTCPNetworkConfiguration(), configMode, (audioConfig.playbackMode & PlaybackMode::INPUT) != 0));
}

void ProcessorRTP::startup()
{
    rtpListener->startUp();
    rtcpHandler->startUp();
}

unsigned int ProcessorRTP::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, ohmcomm::StreamData *userData)
{
    // pack data into a rtp-package
    if (rtpPackage.get() == nullptr)
    {
        initPackageHandler(userData->maxBufferSize);
    }
    if(isDTXEnabled && userData->isSilentPackage)
    {
        //wait a few packages (specified in time, not frames) until not sending anything to prevent too abrupt silence
        ++currentSilenceDelayPackages;
        if(currentSilenceDelayPackages > totalSilenceDelayPackages)
        {
            lastPackageWasSilent = true;
            ohmcomm::debug("RTP") << "Not sending silent package" << ohmcomm::endl;
            return inputBufferByteSize;
        }
    }
    const void* newRTPPackage = rtpPackage->createNewRTPPackage(inputBuffer, inputBufferByteSize);
    if(lastPackageWasSilent)
    {
        //set the marker bit after a silence period
        ((RTPHeader*)newRTPPackage)->setMarker(true);
        lastPackageWasSilent = false;
        currentSilenceDelayPackages = 0;
    }
    //only send the number of bytes really required: header + actual payload-size
    this->network->sendData(newRTPPackage, rtpPackage->getRTPHeaderSize() + inputBufferByteSize);

    ourselves.extendedHighestSequenceNumber += 1;
    ourselves.totalPackages += 1;
    ourselves.totalBytes += rtpPackage->getRTPHeaderSize() + inputBufferByteSize;
    Statistics::incrementCounter(Statistics::COUNTER_FRAMES_SENT, userData->nBufferFrames);
    Statistics::incrementCounter(Statistics::COUNTER_PACKAGES_SENT, 1);
    Statistics::incrementCounter(Statistics::COUNTER_HEADER_BYTES_SENT, rtpPackage->getRTPHeaderSize());
    Statistics::incrementCounter(Statistics::COUNTER_PAYLOAD_BYTES_SENT, inputBufferByteSize);

    //no changes in buffer-size
    return inputBufferByteSize;
}

unsigned int ProcessorRTP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, ohmcomm::StreamData *userData)
{
    // unpack data from a RTP-package
    if (rtpPackage.get() == nullptr)
    {
        initPackageHandler(userData->maxBufferSize);
    }
    //read package from buffer
    //XXX workaround to support one2one conversation with new code
    auto result = buffers.getBuffer((*ParticipantDatabase::getAllRemoteParticipants().begin()).first)->readPackage(*rtpPackage);

    if (result == RTPBufferStatus::RTP_BUFFER_IS_PUFFERING)
    {
        ohmcomm::warn("RTP") <<  "Buffer is buffering" << ohmcomm::endl;
    }
    else if (result == RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW)
    {
        userData->isSilentPackage = true;
        ohmcomm::warn("RTP") << "Output Buffer underflow" << ohmcomm::endl;
    }
    else
    {
        userData->isSilentPackage = false;
    }

    const void* recvAudioData = rtpPackage->getRTPPackageData();
    unsigned int receivedPayloadSize = rtpPackage->getActualPayloadSize();
    memcpy(outputBuffer, recvAudioData, receivedPayloadSize);

    //set received payload size for all following processors to use
    return receivedPayloadSize;
}

bool ProcessorRTP::cleanUp()
{
    //if we never sent a RTP-package, there is no need to end the communication
    if(rtpPackage)
    {
        ohmcomm::info("RTP") << "Communication terminated." << ohmcomm::endl;
    }
    if(rtpListener)
        rtpListener->shutdown();
    if(rtcpHandler)
        rtcpHandler->shutdown();
    //close network anyway
    network->closeNetwork();
    buffers.cleanup();
    return true;
}

void ProcessorRTP::initPackageHandler(unsigned int maxBufferSize)
{
    if(rtpPackage.get() == nullptr)
    {
        rtpPackage.reset(new RTPPackageHandler(maxBufferSize));
    }
    ourselves.initialRTPTimestamp = rtpPackage->initialTimestamp;
    ourselves.extendedHighestSequenceNumber = rtpPackage->sequenceNr;
}
