/* 
 * File:   PassiveConfigurationHandler.cpp
 * Author: daniel
 * 
 * Created on August 29, 2015, 6:04 PM
 */

#include "PassiveConfigurationHandler.h"
#include "RTPPackageHandler.h"
#include "RTCPPackageHandler.h"
#include "UDPWrapper.h"

PassiveConfigurationHandler::PassiveConfigurationHandler(std::shared_ptr<NetworkWrapper> networkWrapper,const std::shared_ptr<ConfigurationMode> config) : 
    configMode(config), networkWrapper(networkWrapper)
{
    listenerThread = std::thread(&PassiveConfigurationHandler::runThread, this);
}

PassiveConfigurationHandler::~PassiveConfigurationHandler()
{
}

void PassiveConfigurationHandler::waitForConfigurationRequest()
{
    listenerThread.join();
}


void PassiveConfigurationHandler::runThread()
{
    std::cout << "Configuration request listener started..." << std::endl;
    RTPPackageHandler rtpHandler(7000);
    RTCPPackageHandler rtcpHandler;
    char* receiveBuffer = (char*)rtpHandler.getWorkBuffer();
    while(true)
    {
        int receivedSize = networkWrapper->receiveData(receiveBuffer, rtpHandler.getMaximumPackageSize());
        if(receivedSize == INVALID_SOCKET)
        {
            //socket was already closed
            break;
        }
        if(!rtcpHandler.isRTCPPackage(receiveBuffer, receivedSize))
        {
            std::cout << "Invalid package-type received" << std::endl;
            continue;
        }
        RTCPHeader header = rtcpHandler.readRTCPHeader(receiveBuffer, receivedSize);
        if(header.packageType != RTCP_PACKAGE_APPLICATION_DEFINED)
        {
            std::cout << "Invalid RTCP package-type received" << std::endl;
            continue;
        }
        ApplicationDefined appDefinedRequest = rtcpHandler.readApplicationDefinedMessage(receiveBuffer, receivedSize, header);
        if(appDefinedRequest.subType == ApplicationDefined::OHMCOMM_CONFIGURATION_REQUEST)
        {
            std::cout << "Configuration-request received" << std::endl;
            //write audio-configuration back to the remote
            PassiveConfiguration::ConfigurationMessage msg;
            const AudioConfiguration audioConfig = configMode->getAudioConfiguration();
            msg.audioFormat = audioConfig.audioFormatFlag;
            msg.bufferFrames = audioConfig.framesPerPackage;
            msg.nChannels = audioConfig.inputDeviceChannels;
            msg.sampleRate = audioConfig.sampleRate;
            configMode->getAudioProcessorsConfiguration(msg.processorNames);
            
            const unsigned int configMessageBufferSize = 512;
            char configMessageBuffer[configMessageBufferSize] = {0};
            ApplicationDefined configResponse("RESC", configMessageBufferSize, configMessageBuffer, ApplicationDefined::OHMCOMM_CONFIGURATION_RESPONSE);
            unsigned int size = PassiveConfiguration::writeConfigurationMessage(configMessageBuffer, configMessageBufferSize, msg);
            if(size < 0)
            {
                std::cerr << "Buffer not large enough!" << std::endl;
                return;
            }
            std::cout << "Sending configuration-response: audio-format = " << AudioConfiguration::getAudioFormatDescription(msg.audioFormat, false) << ", sample-rate = " 
                    << msg.sampleRate << ", channels = " << msg.nChannels << ", number of audio-processors = " << msg.numProcessorNames << std::endl;
            RTCPPackageHandler handler;
            RTCPHeader responseHeader(0);  //SSID doesn't really matter
            void* responseBuffer = handler.createApplicationDefinedPackage(responseHeader, configResponse);
            networkWrapper->sendData(responseBuffer, RTCPPackageHandler::getRTCPPackageLength(responseHeader.length));
            break;
        }
    }
    std::cout << "Configuration request listener shut down..." << std::endl;
}



