/* 
 * File:   PassiveConfiguration.cpp
 * Author: daniel
 * 
 * Created on November 30, 2015, 5:35 PM
 */

#include "config/PassiveConfiguration.h"
#include "audio/AudioHandlerFactory.h"

using namespace ohmcomm;
using namespace ohmcomm::rtp;

//TODO add fallback configuration-mode, because currently, it doesn't support custom-config (and therefore no SDES config)
//problem: fallback config-mode is not fully configured and may throw errors!

PassiveConfiguration::PassiveConfiguration(const NetworkConfiguration& networkConfig, bool profileProcessors, std::string logFile) :
        ConfigurationMode()
{
    this->useDefaultAudioConfig = false;
    this->audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    this->networkConfig = networkConfig;
    this->profileProcessors = profileProcessors;
    this->waitForConfigurationRequest = false;
    if(!logFile.empty())
    {
        this->logToFile = true;
        this->logFileName = logFile;
    }
    else{
        this->logToFile = false;
    }
}

bool PassiveConfiguration::runConfiguration()
{
    if(isConfigurationDone)
    {
        return true;
    }
    //we need to connect to remote RTCP port
    NetworkConfiguration rtcpConfig = networkConfig;
    rtcpConfig.localPort += 1;
    rtcpConfig.remotePort += 1;
    ohmcomm::network::UDPWrapper wrapper(rtcpConfig);

    RTCPPackageHandler handler;
    RTCPHeader requestHeader(0);  //we do not have a SSID and it doesn't really matter
    ApplicationDefined configRequest("REQC", 0, nullptr, ApplicationDefined::OHMCOMM_CONFIGURATION_REQUEST);

    void* buffer = (void*)handler.createApplicationDefinedPackage(requestHeader, configRequest);

    std::cout << "Sending request for passive configuration..." << std::endl;
    if(wrapper.sendData(buffer, RTCPPackageHandler::getRTCPPackageLength(requestHeader.getLength())) < (int)RTCPPackageHandler::getRTCPPackageLength(requestHeader.getLength()))
    {
        std::wcerr << wrapper.getLastError() << std::endl;
        return false;
    }

    //we can reuse buffer, because it is large enough (as of RTCPPackageHandler)
    int receivedSize = wrapper.receiveData(buffer, 6000).status;
    while(receivedSize == ohmcomm::network::NetworkWrapper::RECEIVE_TIMEOUT)
    {
        //we timed out - repeat
        receivedSize = wrapper.receiveData(buffer, 6000).status;
    }
    if(receivedSize < 0)
    {
        std::wcerr << wrapper.getLastError() << std::endl;
        return false;
    }
    else if(receivedSize == 0)
    {
        //we didn't receive anything. Don't know if this can actually occur
        return false;
    }
    RTCPHeader responseHeader = handler.readRTCPHeader(buffer, receivedSize);
    if(responseHeader.getType() != RTCP_PACKAGE_APPLICATION_DEFINED)
    {
        std::cerr << "Invalid RTCP response package type: " << (unsigned int)responseHeader.getType() << std::endl;
        return false;
    }
    ApplicationDefined configResponse = handler.readApplicationDefinedMessage(buffer, receivedSize, responseHeader);
    if(configResponse.subType != ApplicationDefined::OHMCOMM_CONFIGURATION_RESPONSE)
    {
        std::cerr << "Invalid RTCP sub-type for response package: " << configResponse.name << " " << configResponse.subType << std::endl;
        return false;
    }
    //we need to receive audio-configuration and processor-names
    ConfigurationMessage receivedMessage = PassiveConfiguration::readConfigurationMessage(configResponse.data, configResponse.dataLength);

    std::cout << "Passive Configuration received ... " << std::endl;
    
    std::cout << "Received audio-format: " << AudioConfiguration::getAudioFormatDescription(receivedMessage.audioFormat, false) << std::endl;;
    audioConfig.forceAudioFormatFlag = receivedMessage.audioFormat;
    std::cout << "Received sample-rate: " << receivedMessage.sampleRate << std::endl;
    audioConfig.forceSampleRate = receivedMessage.sampleRate;
    std::cout << "Received channels: " << receivedMessage.nChannels << std::endl;
    audioConfig.inputDeviceChannels = receivedMessage.nChannels;
    audioConfig.outputDeviceChannels = receivedMessage.nChannels;
    std::cout << "Received audio-processors: ";
    processorNames.reserve(receivedMessage.processorNames.size());
    for(std::string& procName : receivedMessage.processorNames)
    {
        std::cout << procName << ' ';
        processorNames.push_back(procName);
    }
    std::cout << std::endl;

    this->isConfigurationDone = true;
    return true;
}

const std::string PassiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    return defaultValue;
}

int PassiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    return defaultValue;
}

bool PassiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    return defaultValue;
}

bool PassiveConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    return false;
}


const PassiveConfiguration::ConfigurationMessage PassiveConfiguration::readConfigurationMessage(const void* buffer, unsigned int bufferSize)
{
    ConfigurationMessage message;
    message.audioFormat = ((ConfigurationMessage*)buffer)->audioFormat;
    message.bufferFrames = ((ConfigurationMessage*)buffer)->bufferFrames;
    message.nChannels = ((ConfigurationMessage*)buffer)->nChannels;
    message.numProcessorNames = ((ConfigurationMessage*)buffer)->numProcessorNames;
    message.sampleRate = ((ConfigurationMessage*)buffer)->sampleRate;
    //we filled the vector with trash, so we initialize a new one
    message.processorNames.reserve(message.numProcessorNames);
    //get the pointer to the first processor-name
    char* procNamePtr = (char*)buffer + CONFIGURATION_MESSAGE_SIZE;
    for(uint8_t numProc = 0; numProc < message.numProcessorNames; numProc++)
    {
        std::string nextProcName = std::string(procNamePtr);
        message.processorNames.push_back(nextProcName);
        //skip processor-name and zero-termination
        procNamePtr += nextProcName.size() + 1;
    }
    return message;
}

int PassiveConfiguration::writeConfigurationMessage(void* buffer, unsigned int maxBufferSize, ConfigurationMessage& configMessage)
{
    if(maxBufferSize < CONFIGURATION_MESSAGE_SIZE)
    {
        return -1;
    }
    unsigned int writtenBytes = 0;
    configMessage.numProcessorNames = configMessage.processorNames.size();
    //copy the configuration-message to the output-buffer
    *((ConfigurationMessage*)buffer) = configMessage;
    writtenBytes = CONFIGURATION_MESSAGE_SIZE;
    //we can't send a vector, so we map it to a string-array
    for(uint8_t procIndex = 0; procIndex < configMessage.numProcessorNames; procIndex++)
    {
        std::string nextProcName = configMessage.processorNames[procIndex];
        //copy the string into the package
        writtenBytes += nextProcName.copy((char*)buffer + writtenBytes, nextProcName.size(), 0);
        //terminate name with a zero-byte
        *((char*)buffer + writtenBytes) = '\0';
        writtenBytes += 1;
    }
    return writtenBytes;
}