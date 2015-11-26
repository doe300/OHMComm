/* 
 * File:   RTCPHandler.cpp
 * Author: daniel
 * 
 * Created on November 5, 2015, 11:04 AM
 */

#include <chrono>

#include "rtp/RTCPHandler.h"

const std::chrono::seconds RTCPHandler::sendSRInterval{20};

RTCPHandler::RTCPHandler(std::unique_ptr<NetworkWrapper>&& networkWrapper, const std::shared_ptr<ConfigurationMode> configMode, 
                         const std::function<void ()> startCallback, const std::function<void()> stopCallback):
    wrapper(std::move(networkWrapper)), configMode(configMode), startAudioCallback(startCallback), stopCallback(stopCallback), rtcpHandler(),
        lastSRReceived(std::chrono::milliseconds::zero()), lastSRSent(std::chrono::milliseconds::zero())
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
    sendByePackage();
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
    
    while(threadRunning)
    {
        if(std::chrono::system_clock::now() - sendSRInterval >= lastSRSent)
        {
            //send sender report (SR) every X seconds
            lastSRSent = std::chrono::system_clock::now();
            sendSourceDescription();
        }
        //wait for package and store into RTCPPackageHandler
        int receivedSize = this->wrapper->receiveData(rtcpHandler.rtcpPackageBuffer.data(), rtcpHandler.rtcpPackageBuffer.capacity());
        if(threadRunning == false || receivedSize == INVALID_SOCKET)
        {
            //socket was already closed
            shutdownInternal();
        }
        else if(receivedSize == NetworkWrapper::RECEIVE_TIMEOUT)
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        else if (RTCPPackageHandler::isRTCPPackage(rtcpHandler.rtcpPackageBuffer.data(), receivedSize))
        {
            handleRTCPPackage(rtcpHandler.rtcpPackageBuffer.data(), (unsigned int)receivedSize);
        }
    }
    std::cout << "RTCP-Handler shut down!" << std::endl;
}

void RTCPHandler::handleRTCPPackage(void* receiveBuffer, unsigned int receivedSize)
{
    //handle RTCP-packages
    RTCPHeader header = rtcpHandler.readRTCPHeader(receiveBuffer, receivedSize);
    if(header.getType() == RTCP_PACKAGE_GOODBYE)
    {
        //other side sent an BYE-package, shutting down
        std::cout << "RTCP: Received Goodbye-message: " << rtcpHandler.readByeMessage(receiveBuffer, receivedSize, header) << std::endl;
        std::cout << "RTCP: Dialog partner requested end of communication, shutting down!" << std::endl;
        shutdownInternal();
        //notify OHMComm to shut down
        stopCallback();
        return;
    }
    else if(header.getType() == RTCP_PACKAGE_SENDER_REPORT)
    {
        //other side sent SR, so print output
        lastSRReceived = std::chrono::system_clock::now();
        NTPTimestamp ntpTime;
        SenderInformation senderReport(ntpTime, 0, 0,0);
        std::vector<ReceptionReport> receptionReports = rtcpHandler.readSenderReport(receiveBuffer, receivedSize, header, senderReport);
        std::cout << "RTCP: Received Sender Report: " << std::endl;
        std::cout << "\tTotal package sent: " << senderReport.getPacketCount() << std::endl;
        std::cout << "\tTotal bytes sent: " << senderReport.getOctetCount() << std::endl;
        std::cout << "RTCP: Received Reception Reports:" << std::endl;
        for(const ReceptionReport& report : receptionReports)
        {
            std::cout << "\tReception Report for: " << report.getSSRC() << std::endl;
            std::cout << "\t\tFraction Lost (1/256): " << (unsigned int)report.getFractionLost() << std::endl;
            std::cout << "\t\tTotal package loss: " << report.getCummulativePackageLoss() << std::endl;
            std::cout << "\t\tInterarrival Jitter (in ms): " << report.getInterarrivalJitter() << std::endl;
        }
    }
    else if(header.getType() == RTCP_PACKAGE_SOURCE_DESCRIPTION)
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
    else if(header.getType() == RTCP_PACKAGE_APPLICATION_DEFINED)
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
            const void* responseBuffer = handler.createApplicationDefinedPackage(responseHeader, configResponse);
            wrapper->sendData(responseBuffer, RTCPPackageHandler::getRTCPPackageLength(responseHeader.getLength()));
            
            //remote device has connected, so send SDES
            sendSourceDescription();
            
            //start audio-playback
            startAudioCallback();
        }
    }
    else
    {
        std::cerr << "RTCP: Unrecognized package-type: " << (unsigned int)header.getType() << std::endl;
        return;
    }
    
    //handle RTCP compound packages
    unsigned int packageLength = RTCPPackageHandler::getRTCPPackageLength(header.getLength());
    void* remainingBuffer = (char*)receiveBuffer + packageLength;
    unsigned int remainingLength = receivedSize - packageLength;
    if(RTCPPackageHandler::isRTCPPackage(remainingBuffer, remainingLength))
    {
        handleRTCPPackage(remainingBuffer, remainingLength);
    }
}

void RTCPHandler::sendSourceDescription()
{
    std::cout << "RTCP: Sending SR + SDES ..." << std::endl;
    const void* buffer = createSenderReport();
    unsigned int length = RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)buffer)->getLength());
    const void* tmp = createSourceDescription(length);
    length += RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)tmp)->getLength());
    wrapper->sendData(buffer, length);
}

void RTCPHandler::sendByePackage()
{
    std::cout << "RTCP: Sending SR + SDES + BYE ..." << std::endl;
    const void* buffer = createSenderReport();
    unsigned int length = RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)buffer)->getLength());
    const void* tmp = createSourceDescription(length);
    length += RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)tmp)->getLength());
    //create bye package
    RTCPHeader byeHeader(participantDatabase[PARTICIPANT_SELF].ssrc);
    rtcpHandler.createByePackage(byeHeader, "Program exit", length);
    length += RTCPPackageHandler::getRTCPPackageLength(byeHeader.getLength());
    wrapper->sendData(buffer, length);
    
    std::cout << "RTCP: BYE-Package sent." << std::endl;
}


inline uint8_t RTCPHandler::calculateFractionLost()
{
    double c = Statistics::readCounter(Statistics::COUNTER_PACKAGES_LOST);
    double d = Statistics::readCounter(Statistics::COUNTER_PACKAGES_RECEIVED);
    return (long)((c/d) * 256);
}

const void* RTCPHandler::createSenderReport(unsigned int offset)
{
    RTCPHeader srHeader(participantDatabase[PARTICIPANT_SELF].ssrc);
    
    NTPTimestamp ntpTime = NTPTimestamp::now();
    const std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    const uint32_t rtpTimestamp = participantDatabase[PARTICIPANT_SELF].initialRTPTimestamp + now.count();
    const std::chrono::milliseconds lastSRTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(lastSRReceived.time_since_epoch());
    
    SenderInformation senderReport(ntpTime, rtpTimestamp, Statistics::readCounter(Statistics::COUNTER_PACKAGES_SENT), Statistics::readCounter(Statistics::COUNTER_PAYLOAD_BYTES_SENT));
    //we currently have only one reception report
    ReceptionReport receptionReport;
    receptionReport.setSSRC(participantDatabase[PARTICIPANT_REMOTE].ssrc);
    receptionReport.setFractionLost(calculateFractionLost());
    receptionReport.setCummulativePackageLoss((uint32_t)Statistics::readCounter(Statistics::COUNTER_PACKAGES_LOST));
    receptionReport.setExtendedHighestSequenceNumber(participantDatabase[PARTICIPANT_REMOTE].extendedHighestSequenceNumber);
    receptionReport.setInterarrivalJitter((uint32_t)round(participantDatabase[PARTICIPANT_REMOTE].interarrivalJitter));
    receptionReport.setLastSRTimestamp((uint32_t)lastSRTimestamp.count());
    //XXX delay since last SR /maybe last SR is wrong
    receptionReport.setDelaySinceLastSR((now - lastSRTimestamp).count());
    
    std::vector<ReceptionReport> receptionReports;
    if(receptionReport.getSSRC() != 0)
    {
        //we can skip initial reception-report if we don't even know for whom
        receptionReports.push_back(receptionReport);
    }
    return rtcpHandler.createSenderReportPackage(srHeader, senderReport, receptionReports, offset);
}

const void* RTCPHandler::createSourceDescription(unsigned int offset)
{
    std::vector<SourceDescription> sdes = {
        {RTCP_SOURCE_TOOL, std::string("OHMComm v") + OHMCOMM_VERSION},
		{RTCP_SOURCE_CNAME, (getUserName() + '@') + getDomainName()}
    };
    //TODO clashes with interactive configuration (with input to shutdown server)
    if(std::dynamic_pointer_cast<InteractiveConfiguration>(configMode) == nullptr && configMode->isConfigured())
    {
        //add user configured values
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
    RTCPHeader sdesHeader(participantDatabase[PARTICIPANT_SELF].ssrc);
    return rtcpHandler.createSourceDescriptionPackage(sdesHeader, sdes, offset);
}