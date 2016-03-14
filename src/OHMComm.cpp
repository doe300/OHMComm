/*
 * File:   OHMComm.cpp
 * Author: daniel
 *
 * Created on August 17, 2015, 4:42 PM
 */

#include "OHMComm.h"
#include "rtp/ProcessorRTP.h"
#include "processors/AudioProcessorFactory.h"
#include "audio/AudioHandlerFactory.h"
#include "config/ConfigurationMode.h"

using namespace ohmcomm;

OHMComm::OHMComm(ConfigurationMode* mode) : configurationMode(mode), audioHandler(nullptr)
{
    registerPlaybackListener(configurationMode);
    rtp::ParticipantDatabase::registerListener(*this);
}

OHMComm::~OHMComm()
{
    rtp::ParticipantDatabase::unregisterListener(*this);
}

std::shared_ptr<ConfigurationMode> OHMComm::getConfigurationMode() const
{
    return configurationMode;
}

bool OHMComm::isConfigurationActive() const
{
    return configurationActive;
}

bool OHMComm::isConfigurationDone(bool runConfiguration)
{
    if(!configurationActive)
    {
        return true;
    }
    if(configurationMode->isConfigured())
    {
        return true;
    }
    if(runConfiguration)
    {
        //run interactive configuration
        return configurationMode->runConfiguration();
    }
    return false;
}

void OHMComm::startAudioThreads()
{
    if(running)
    {
        std::cout << "OHMComm already running..." << std::endl;
        return;
    }
    if(!isConfigurationDone(false))
    {
        throw std::runtime_error("OHMComm not fully configured!");
    }
    configurationActive = false;
    //initialize audio-objects
    if(configurationMode->getAudioHandlerConfiguration().second)
    {
        audioHandler = AudioHandlerFactory::getAudioHandler(configurationMode->getAudioHandlerConfiguration().first, configurationMode->getAudioConfiguration());
    }
    else
    {
        audioHandler = AudioHandlerFactory::getAudioHandler(configurationMode->getAudioHandlerConfiguration().first);
    }
    std::vector<std::string> procNames(0);
    bool profileProcessors = configurationMode->getAudioProcessorsConfiguration(procNames);
    PayloadType payloadType = PayloadType::L16_2;
    for(const std::string& procName : procNames)
    {
        AudioProcessor* proc = AudioProcessorFactory::getAudioProcessor(procName, profileProcessors);
        audioHandler->getProcessors().addProcessor(proc);
        //only the last non-default payload-type is required
        payloadType = proc->getSupportedPlayloadType() == PayloadType::ALL ? payloadType : proc->getSupportedPlayloadType();
    }
    if(configurationMode->getPayloadType() != PayloadType::ALL)
    {
        //if we use a custom payload-type (e.g. for SIP-config), it must be set here
        payloadType = (PayloadType)configurationMode->getPayloadType();
    }
    configureRTPProcessor(profileProcessors, payloadType);

    //run threads

    if(!audioHandler->prepare(configurationMode))
    {
        throw std::runtime_error("Failed to configure audio-handler!");
    }
    //write configuration back to ConfigurationMode
    configurationMode->updateAudioConfiguration(audioHandler->getAudioConfiguration());

    //start RTCP-handler
    rtcpHandler.reset(new rtp::RTCPHandler(configurationMode->getRTCPNetworkConfiguration(), configurationMode, 
            std::bind(&OHMComm::startAudio,this), (audioHandler->getMode() & AudioHandler::INPUT) == AudioHandler::INPUT));
    registerPlaybackListener(rtcpHandler);
    rtcpHandler->startUp();
    //if we don't wait for configuration, begin audio-playback
    //otherwise, the #startAudio()-method is called from the RTCPHandler
    if(!configurationMode->isWaitForConfigurationRequest())
    {
        startAudio();
    }
}

void OHMComm::startAudio()
{
    notifyPlaybackStart();
    if((audioHandler->getMode() & AudioHandler::DUPLEX) == AudioHandler::DUPLEX)
    {
        std::cout << "OHMComm: starting duplex mode ..." << std::endl;
        audioHandler->start(AudioHandler::DUPLEX);
    }
    else if((audioHandler->getMode() & AudioHandler::OUTPUT) == AudioHandler::OUTPUT)
    {
        std::cout << "OHMComm: starting playback mode ..." << std::endl;
        audioHandler->start(AudioHandler::OUTPUT);
    }
    else if((audioHandler->getMode() & AudioHandler::INPUT) == AudioHandler::INPUT)
    {
        std::cout << "OHMComm: starting recording mode ..." << std::endl;
        audioHandler->start(AudioHandler::INPUT);
    }
    else
    {
        std::cerr << "OHMComm: No supported playback-mode configured!" << std::endl;
        return;
    }
    
    std::cout << "OHMComm started!" << std::endl;
    running = true;
}

void OHMComm::stopAudioThreads()
{
    if(!running)
    {
        //to prevent recursive calls
        std::cout << "OHMComm is not running" << std::endl;
        return;
    }
    running = false;

    audioHandler->stop();
    notifyPlaybackStop();
    
    //give threads some time to shut down
    //this is mostly to prevent from parallel logging to console
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::cout << "OHMComm stopped!" << std::endl;
    const std::pair<bool, std::string> logConfig = configurationMode->getLogToFileConfiguration();
    if(logConfig.first)
    {
        Statistics::printStatisticsToFile(logConfig.second);
        std::cout << "Statistics file written" << std::endl;
    }
    //print statistics to stdout anyway
    Statistics::printStatistics();
    
    //if this method was called from one of the background-tasks, only the main-thread (waiting for input) remains.
    std::cout << "Type Enter to exit..." << std::endl;
}

bool OHMComm::isRunning() const
{
    return running;
}

void OHMComm::onRemoteRemoved(const unsigned int ssrc)
{
    if(rtp::ParticipantDatabase::getNumberOfRemotes() == 0)
    {
        std::cout << "OHMComm: No more communication partners, shutting down..." << std::endl;
        stopAudioThreads();
    }
}

void OHMComm::configureRTPProcessor(bool profileProcessors, const PayloadType payloadType)
{
    rtp::ProcessorRTP* rtpProcessor = new rtp::ProcessorRTP("RTP-Processor", configurationMode->getNetworkConfiguration(), payloadType);
    if(profileProcessors)
    {
        //enabled profiling of the RTP processor
        audioHandler->getProcessors().addProcessor(new ProfilingAudioProcessor(rtpProcessor));
    }
    else
    {
        audioHandler->getProcessors().addProcessor(rtpProcessor);
    }
}

std::function<void ()> OHMComm::createStopCallback()
{
    return std::bind(&OHMComm::stopAudioThreads,this);
}