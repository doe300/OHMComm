/* 
 * File:   SIPConfiguration.cpp
 * Author: daniel
 * 
 * Created on December 2, 2015, 1:06 PM
 */

#include "sip/SIPConfiguration.h"
#include "AudioHandlerFactory.h"

#include <chrono>

SIPConfiguration::SIPConfiguration(const NetworkConfiguration& sipConfig, bool profileProcessors, const std::string& logFile) : 
    ConfigurationMode(), handler(sipConfig, "remote", [this](const MediaDescription& media){this->setAudioConfig(media);})
{
    useDefaultAudioConfig = false;
    audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    networkConfig.localPort = DEFAULT_NETWORK_PORT;
    networkConfig.remoteIPAddress = sipConfig.remoteIPAddress;
    profileProcessors = profileProcessors;
    waitForConfigurationRequest = false;
    if(!logFile.empty())
    {
        logToFile = true;
        logFileName = logFile;
    }
    else
    {
        logToFile = false;
    }
}

SIPConfiguration::~SIPConfiguration()
{
    handler.shutdown();
}

bool SIPConfiguration::runConfiguration()
{
    if(isConfigurationDone)
    {
        return true;
    }
    handler.startUp();
    
    int timeLeft = SIPConfiguration::MAX_WAIT_TIME;
    
    //wait for configuration to be done
    while(!isConfigurationDone && handler.isRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // maximum time to wait before aborting configuration
        timeLeft -= 100;
        if(timeLeft <= 0)
        {
            //when aborting, let SIPHandler send CANCEL
            handler.shutdown();
            break;
        }
    }
    return isConfigurationDone;
}

const std::string SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    //TODO how to support custom configuration??
    return defaultValue;
}

const int SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    return defaultValue;
}

const bool SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    return defaultValue;
}

const bool SIPConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    return false;
}

void SIPConfiguration::onPlaybackStop()
{
    handler.shutdown();
}

void SIPConfiguration::setAudioConfig(const MediaDescription& media)
{
    networkConfig.remotePort = media.port;
    audioConfig.forceSampleRate = media.sampleRate;
    audioConfig.inputDeviceChannels = media.numChannels;
    audioConfig.outputDeviceChannels = media.numChannels;
    payloadType = media.payloadType;
    SupportedFormat format = media.getFormat();
    processorNames.push_back(format.processorName);
    
    isConfigurationDone = true;
}