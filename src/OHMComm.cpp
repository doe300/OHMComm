/*
 * File:   OHMComm.cpp
 * Author: daniel
 *
 * Created on August 17, 2015, 4:42 PM
 */

#include "OHMComm.h"
#include "network/UDPWrapper.h"
#include "rtp/ProcessorRTP.h"
#include "AudioProcessorFactory.h"
#include "AudioHandlerFactory.h"
#include "UserInput.h"
#include "ConfigurationMode.h"
#include "rtp/RTCPHandler.h"

OHMComm::OHMComm(ConfigurationMode* mode)
    : rtpBuffer(new RTPBuffer(256, 100, 0)), configurationMode(mode), audioHandler(nullptr), networkWrapper(nullptr), listener(nullptr)
{
    registerPlaybackListener(configurationMode);
}

OHMComm::~OHMComm()
{
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
    networkWrapper.reset(new UDPWrapper(configurationMode->getNetworkConfiguration()));
    std::vector<std::string> procNames(0);
    bool profileProcessors = configurationMode->getAudioProcessorsConfiguration(procNames);
    PayloadType payloadType = PayloadType::L16_2;
    for(const std::string& procName : procNames)
    {
        AudioProcessor* proc = AudioProcessorFactory::getAudioProcessor(procName, profileProcessors);
        audioHandler->addProcessor(proc);
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
    rtcpHandler.reset(new RTCPHandler(std::unique_ptr<NetworkWrapper>(
            new UDPWrapper(configurationMode->getRTCPNetworkConfiguration())), configurationMode, 
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
    listener.reset(new RTPListener(networkWrapper, rtpBuffer, audioHandler->getBufferSize()));
    registerPlaybackListener(listener);
    notifyPlaybackStart();
    if((audioHandler->getMode() & AudioHandler::DUPLEX) == AudioHandler::DUPLEX)
    {
        std::cout << "OHMComm: starting duplex mode ..." << std::endl;
        audioHandler->startDuplexMode();
    }
    else if((audioHandler->getMode() & AudioHandler::OUTPUT) == AudioHandler::OUTPUT)
    {
        std::cout << "OHMComm: starting playback mode ..." << std::endl;
        audioHandler->startPlaybackMode();
    }
    else if((audioHandler->getMode() & AudioHandler::INPUT) == AudioHandler::INPUT)
    {
        std::cout << "OHMComm: starting recording mode ..." << std::endl;
        audioHandler->startRecordingMode();
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
    //we must close this after shutting down threads
    networkWrapper->closeNetwork();
    
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

void OHMComm::configureRTPProcessor(bool profileProcessors, const PayloadType payloadType)
{
    ProcessorRTP* rtpProcessor = new ProcessorRTP("RTP-Processor", networkWrapper, rtpBuffer, payloadType);
    if(profileProcessors)
    {
        //enabled profiling of the RTP processor
        audioHandler->addProcessor(new ProfilingAudioProcessor(rtpProcessor));
    }
    else
    {
        audioHandler->addProcessor(rtpProcessor);
    }
}

std::function<void ()> OHMComm::createStopCallback()
{
    return std::bind(&OHMComm::stopAudioThreads,this);
}