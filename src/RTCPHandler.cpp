/* 
 * File:   RTCPHandler.cpp
 * Author: daniel
 * 
 * Created on November 5, 2015, 11:04 AM
 */

#include "RTCPHandler.h"

//TODO somehow make SSRC known to RTCPHandler (create in OHMComm??)

RTCPHandler::RTCPHandler(std::unique_ptr<NetworkWrapper>&& networkWrapper, const std::shared_ptr<ConfigurationMode> configMode, 
                         const std::function<void ()> startCallback, const std::function<void()> stopCallback):
    wrapper(std::move(networkWrapper)), configMode(configMode), startAudioCallback(startCallback), stopCallback(stopCallback), rtcpHandler()
{
}

RTCPHandler::~RTCPHandler()
{
    // Wait until thread has really stopped
    listenerThread.join();
}

void RTCPHandler::startUp()
{
    threadRunning = true;
    listenerThread = std::thread(&RTCPHandler::runThread, this);
}

void RTCPHandler::shutdown()
{
    // Send a RTCP BYE-packet, to tell the other side that communication has been stopped
    RTCPHeader byeHeader(0); //SSID doesn't really matter, at least for two-user communication
    void* packageBuffer = rtcpHandler.createByePackage(byeHeader, "Program exit");
    wrapper->sendData(packageBuffer, RTCPPackageHandler::getRTCPPackageLength(byeHeader.length));
    std::cout << "RTCP: BYE-Package sent." << std::endl;
    
    shutdownInternal();
}

void RTCPHandler::shutdownInternal()
{
    // notify the thread to stop
    threadRunning = false;
    // close the socket
    wrapper->closeNetwork();
}


void RTCPHandler::runThread()
{
    std::cout << "RTCP-Handler started ..." << std::endl;
    //on startup, send SDES
    sendSourceDescription();
    
    while(threadRunning)
    {
        //1. wait for package and store into RTCPPackageHandler
        int receivedSize = this->wrapper->receiveData(rtcpHandler.rtcpPackageBuffer, rtcpHandler.maxPackageSize);
        if(threadRunning == false || receivedSize == INVALID_SOCKET)
        {
            //socket was already closed
            shutdownInternal();
        }
        else if (RTCPPackageHandler::isRTCPPackage(rtcpHandler.rtcpPackageBuffer, receivedSize))
        {
            handleRTCPPackage(rtcpHandler.rtcpPackageBuffer, (unsigned int)receivedSize);
        }
        else if(receivedSize == EAGAIN || receivedSize == EWOULDBLOCK)
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
    }
    std::cout << "RTCP-Handler shut down!" << std::endl;
}

void RTCPHandler::handleRTCPPackage(void* receiveBuffer, unsigned int receivedSize)
{
    //handle RTCP-packages
    RTCPHeader header = rtcpHandler.readRTCPHeader(receiveBuffer, receivedSize);
    if(header.packageType == RTCP_PACKAGE_GOODBYE)
    {
        //other side sent an BYE-package, shutting down
        std::cout << "RTCP: Received Goodbye-message: " << rtcpHandler.readByeMessage(receiveBuffer, receivedSize, header) << std::endl;
        std::cout << "RTCP: Dialog partner requested end of communication, shutting down!" << std::endl;
        shutdownInternal();
        //notify OHMComm to shut down
        stopCallback();
    }
    else if(header.packageType == RTCP_PACKAGE_SOURCE_DESCRIPTION)
    {
        //other side sent SDES, so print to output
        std::vector<SourceDescription> sourceDescriptions = rtcpHandler.readSourceDescription(receiveBuffer, receivedSize, header);
        std::cout << "RTCP: Received Source Description:" << std::endl;
        for(const SourceDescription& descr : sourceDescriptions)
        {
            std::cout << "\t" << descr.getTypeName() << ": " << descr.value << std::endl;
        }
        std::cout << std::endl;
    }
    else if(header.packageType == RTCP_PACKAGE_APPLICATION_DEFINED)
    {
        //other side sent AD, assume it is an configuration-request
        ApplicationDefined appDefinedRequest = rtcpHandler.readApplicationDefinedMessage(receiveBuffer, receivedSize, header);
        if(appDefinedRequest.subType == ApplicationDefined::OHMCOMM_CONFIGURATION_REQUEST)
        {
            std::cout << "RTCP: Configuration-request received" << std::endl;
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
            std::cout << "RTCP: Sending configuration-response: audio-format = " << AudioConfiguration::getAudioFormatDescription(msg.audioFormat, false) << ", sample-rate = " 
                    << msg.sampleRate << ", channels = " << msg.nChannels << ", number of audio-processors = " << msg.numProcessorNames << std::endl;
            RTCPPackageHandler handler;
            RTCPHeader responseHeader(0);  //SSID doesn't really matter
            void* responseBuffer = handler.createApplicationDefinedPackage(responseHeader, configResponse);
            wrapper->sendData(responseBuffer, RTCPPackageHandler::getRTCPPackageLength(responseHeader.length));
            
            //remote device has connected, so send SDES
            sendSourceDescription();
            
            //start audio-playback
            startAudioCallback();
        }
    }
    else
    {
        std::cerr << "RTCP: Unrecognized package-type: " << (unsigned int)header.packageType << std::endl;
    }
}

void RTCPHandler::sendSourceDescription()
{
    std::vector<SourceDescription> sdes = {
        {RTCP_SOURCE_TOOL, std::string("OHMComm v") + OHMCOMM_VERSION}
    };
    //TODO clashes with interactive configuration (with input to shutdown server)
    if(std::dynamic_pointer_cast<InteractiveConfiguration>(configMode) == nullptr && configMode->isConfigured())
    {
        //add user configured values
        if(configMode->isCustomConfigurationSet(Parameters::SDES_CNAME->longName, "SDES CNAME?"))
        {
            sdes.emplace_back(RTCP_SOURCE_CNAME, configMode->getCustomConfiguration(Parameters::SDES_CNAME->longName, "Enter SDES CNAME", "Generic device"));
        }
        if(configMode->isCustomConfigurationSet(Parameters::SDES_EMAIL->longName, "SDES EMAIL?"))
        {
            sdes.emplace_back(RTCP_SOURCE_EMAIL, configMode->getCustomConfiguration(Parameters::SDES_EMAIL->longName, "Enter SDES EMAIL", "anon@noreply.com"));
        }
        if(configMode->isCustomConfigurationSet(Parameters::SDES_LOC->longName, "SDES LOCATION?"))
        {
            sdes.emplace_back(RTCP_SOURCE_LOC, configMode->getCustomConfiguration(Parameters::SDES_LOC->longName, "Enter SDES LOCATION", "earth"));
        }
        if(configMode->isCustomConfigurationSet(Parameters::SDES_NAME->longName, "SDES NAME?"))
        {
            sdes.emplace_back(RTCP_SOURCE_NAME, configMode->getCustomConfiguration(Parameters::SDES_NAME->longName, "Enter SDES NAME", "anon"));
        }
        if(configMode->isCustomConfigurationSet(Parameters::SDES_NOTE->longName, "SDES NOTE?"))
        {
            sdes.emplace_back(RTCP_SOURCE_NOTE, configMode->getCustomConfiguration(Parameters::SDES_NOTE->longName, "Enter SDES NOTE", ""));
        }
        if(configMode->isCustomConfigurationSet(Parameters::SDES_PHONE->longName, "SDES PHONE?"))
        {
            sdes.emplace_back(RTCP_SOURCE_PHONE, configMode->getCustomConfiguration(Parameters::SDES_PHONE->longName, "Enter SDES PHONE", ""));
        }
    }
    std::cout << "RTCP: sending SDES ..." << std::endl;
    RTCPHeader sdesHeader(0);
    void* buffer = rtcpHandler.createSourceDescriptionPackage(sdesHeader, sdes);
    wrapper->sendData(buffer, RTCPPackageHandler::getRTCPPackageLength(sdesHeader.length));
}
