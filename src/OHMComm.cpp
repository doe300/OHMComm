/*
 * File:   OHMComm.cpp
 * Author: daniel
 *
 * Created on August 17, 2015, 4:42 PM
 */

#include "OHMComm.h"
#include "UDPWrapper.h"
#include "ProcessorRTP.h"
#include "AudioProcessorFactory.h"
#include "AudioHandlerFactory.h"
#include "UserInput.h"
#include "RtAudio.h"
#include "ConfigurationMode.h"
#include "RTPBufferAlternative.h"

OHMComm::OHMComm(ConfigurationMode* mode)
    : rtpBuffer(new RTPBuffer(256, 1000, 32)), configurationMode(mode), audioHandler(nullptr), networkWrapper(nullptr), listener(nullptr)
{
}

OHMComm::~OHMComm()
{
    audioHandler.reset(nullptr);
    listener.reset(nullptr);
    networkWrapper.reset();
    rtpBuffer.reset();
    delete configurationMode;
}

ConfigurationMode* OHMComm::getConfigurationMode() const
{
    return configurationMode;
}

const bool OHMComm::isConfigurationActive() const
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
    std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(configurationMode->getNetworkConfiguration()));
    networkWrapper = std::move(tmp);
    std::vector<std::string> procNames(0);
    bool profileProcessors = configurationMode->getAudioProcessorsConfiguration(procNames);
    for(const std::string& procName : procNames)
    {
        audioHandler->addProcessor(AudioProcessorFactory::getAudioProcessor(procName, profileProcessors));
    }
    configureRTPProcessor(profileProcessors);

    //run threads

    if(!audioHandler->prepare())
    {
        throw std::runtime_error("Failed to configure audio-handler!");
    }
    listener.reset(new RTPListener(networkWrapper, rtpBuffer, audioHandler->getBufferSize(), configurationMode, createStopCallback()));

    listener->startUp();
    audioHandler->startDuplexMode();

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
    listener->shutdown();
    networkWrapper->closeNetwork();

    std::cout << "OHMComm stopped!" << std::endl;
    const std::pair<bool, std::string> logConfig = configurationMode->getLogToFileConfiguration();
    if(logConfig.first)
    {
        Statistics::printStatisticsToFile(logConfig.second);
        std::cout << "Statistics file written" << std::endl;
    }
    //print statistics to stdout anyway
    Statistics::printStatistics();
}

bool OHMComm::isRunning() const
{
    return running;
}

void OHMComm::configureRTPProcessor(bool profileProcessors)
{
    ProcessorRTP* rtpProcessor = new ProcessorRTP("RTP-Processor", networkWrapper, rtpBuffer);
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