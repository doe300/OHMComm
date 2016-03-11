/* 
 * File:   RTCPHandler.cpp
 * Author: daniel
 * 
 * Created on November 5, 2015, 11:04 AM
 */

#include <cmath>    //round

#include "rtp/RTCPHandler.h"
#include "config/InteractiveConfiguration.h"
#include "config/PassiveConfiguration.h"
#include "Parameters.h"
#include "Utility.h"

//standard-conform minimum interval of 5 seconds (no need to be adaptive, as long as we only have one remote) (RFC3550 Section 6.2)
const std::chrono::seconds RTCPHandler::sendSRInterval{5};
const std::chrono::seconds RTCPHandler::remoteDropoutTimeout{60};

RTCPHandler::RTCPHandler(const NetworkConfiguration& rtcpConfig, const std::shared_ptr<ConfigurationMode> configMode, 
                         const std::function<void ()> startCallback, const bool isActiveSender):
    wrapper(new UDPWrapper(rtcpConfig)), configMode(configMode), startAudioCallback(startCallback),
        isActiveSender(isActiveSender), rtcpHandler(), ourselves(ParticipantDatabase::self())
{
    //make sure, RTCP for self is set
    if(!ourselves.rtcpData)
        ourselves.rtcpData.reset(new RTCPData);
}

RTCPHandler::~RTCPHandler()
{
    // Wait until thread has really stopped
    listenerThread.join();
}

void RTCPHandler::startUp()
{
    if(!threadRunning)
    {
        threadRunning = true;
        listenerThread = std::thread(&RTCPHandler::runThread, this);
    }
}

void RTCPHandler::shutdown()
{
    if(threadRunning)
    {
        // Send a RTCP BYE-packet, to tell the other side that communication has been stopped
        sendByePackage();
        shutdownInternal();
    }
}

void RTCPHandler::onRegister(PlaybackObservee* ohmComm)
{
    stopCallback = ohmComm->createStopCallback();
}

void RTCPHandler::onPlaybackStart()
{
    startUp();
}

void RTCPHandler::onPlaybackStop()
{
    shutdown();
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
        if((std::chrono::steady_clock::now() - sendSRInterval) >= ourselves.rtcpData->lastSRTimestamp)
        {
            const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            const auto allParticipants = ParticipantDatabase::getAllRemoteParticipants();
            for(auto it = allParticipants.begin(); it != allParticipants.end(); ++it)
            {
                if(now - (*it).second.lastPackageReceived > remoteDropoutTimeout)
                {
                    //remote has not send any package for quite some time, end conversation
                    //TODO different action + remove participant from DB (shut down all its processors/classes)
                    std::cout << "RTCP: Dialog partner has timed out, shutting down!" << std::endl;
                    shutdownInternal();
                    //notify OHMComm to shut down
                    stopCallback();
                    break;
                }
            }
            //send report (SR/RR) every X seconds
            ourselves.rtcpData->lastSRTimestamp = std::chrono::steady_clock::now();
            sendSourceDescription();
        }
        //wait for package and store into RTCPPackageHandler
        const NetworkWrapper::Package result = this->wrapper->receiveData(rtcpHandler.rtcpPackageBuffer.data(), rtcpHandler.rtcpPackageBuffer.capacity());
        if(threadRunning == false || result.isInvalidSocket())
        {
            //socket was already closed
            shutdownInternal();
        }
        else if(result.hasTimedOut())
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        else if (RTCPPackageHandler::isRTCPPackage(rtcpHandler.rtcpPackageBuffer.data(), result.getReceivedSize()))
        {
            Statistics::incrementCounter(Statistics::RTCP_PACKAGES_RECEIVED);
            Statistics::incrementCounter(Statistics::RTCP_BYTES_RECEIVED, result.getReceivedSize());
            const uint32_t ssrc = handleRTCPPackage(rtcpHandler.rtcpPackageBuffer.data(), result.getReceivedSize());
            //since every participant (at least using OHMComm) sends SR first on joining conversation,
            //this call to remote(ssrc) is very likely to create the new participant
            ParticipantDatabase::remote(ssrc).lastPackageReceived = std::chrono::steady_clock::now();
        }
    }
    std::cout << "RTCP-Handler shut down!" << std::endl;
}

uint32_t RTCPHandler::handleRTCPPackage(const void* receiveBuffer, unsigned int receivedSize)
{
    //handle RTCP-packages
    RTCPHeader header = rtcpHandler.readRTCPHeader(receiveBuffer, receivedSize);
    Participant& participant = ParticipantDatabase::remote(header.getSSRC());
    if(header.getType() == RTCP_PACKAGE_GOODBYE)
    {
        //other side sent an BYE-package, shutting down
        std::cout << "RTCP: Received Goodbye-message: " << rtcpHandler.readByeMessage(receiveBuffer, receivedSize, header) << std::endl;
        std::cout << "RTCP: Dialog partner requested end of communication, shutting down!" << std::endl;
        shutdownInternal();
        //notify OHMComm to shut down
        stopCallback();
        return header.getSSRC();
    }
    else if(header.getType() == RTCP_PACKAGE_SENDER_REPORT)
    {
        //make sure, RTCP-data pointer is set
        if(!participant.rtcpData)
            participant.rtcpData.reset(new RTCPData);
        //other side sent SR, so print output
        participant.rtcpData->lastSRTimestamp = std::chrono::steady_clock::now();
        NTPTimestamp ntpTime;
        SenderInformation senderReport(ntpTime, 0, 0,0);
        std::vector<ReceptionReport> receptionReports = rtcpHandler.readSenderReport(receiveBuffer, receivedSize, header, senderReport);
        std::cout << "RTCP: Received Sender Report: " << std::endl;
        std::cout << "\tTotal package sent: " << senderReport.getPacketCount() << std::endl;
        std::cout << "\tTotal bytes sent: " << senderReport.getOctetCount() << std::endl;
        printReceptionReports(receptionReports);
    }
    else if(header.getType() == RTCP_PACKAGE_RECEIVER_REPORT)
    {
        //other side sent RR, so print output
        std::vector<ReceptionReport> receptionReports = rtcpHandler.readReceiverReport(receiveBuffer, receivedSize, header);
        std::cout << "RTCP: Received Receiver Report: " << std::endl;
        printReceptionReports(receptionReports);
    }
    else if(header.getType() == RTCP_PACKAGE_SOURCE_DESCRIPTION)
    {
        //make sure, RTCP-data pinter is set
        if(!participant.rtcpData)
            participant.rtcpData.reset(new RTCPData);
        //other side sent SDES, so print to output
        std::vector<SourceDescription> sourceDescriptions = rtcpHandler.readSourceDescription(receiveBuffer, receivedSize, header);
        //set remote source descriptions
        participant.rtcpData->sourceDescriptions = sourceDescriptions;
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
            const int size = PassiveConfiguration::writeConfigurationMessage(configMessageBuffer, configMessageBufferSize, msg);
            if(size < 0)
            {
                std::cerr << "Buffer not large enough!" << std::endl;
                return header.getSSRC();
            }
            std::cout << "RTCP: Sending configuration-response: audio-format = " << AudioConfiguration::getAudioFormatDescription(msg.audioFormat, false) << ", sample-rate = " 
                    << msg.sampleRate << ", channels = " << msg.nChannels << ", number of audio-processors = " << msg.numProcessorNames << std::endl;
            RTCPPackageHandler handler;
            RTCPHeader responseHeader(ourselves.ssrc);
            const void* responseBuffer = handler.createApplicationDefinedPackage(responseHeader, configResponse);
            wrapper->sendData(responseBuffer, RTCPPackageHandler::getRTCPPackageLength(responseHeader.getLength()));
            
            //remote device has connected, so send SDES
            sendSourceDescription();
            
            //XXX could be extended to allow for multiple remotes to request configuration (group call)
            //start audio-playback
            startAudioCallback();
        }
    }
    else
    {
        std::cerr << "RTCP: Unrecognized package-type: " << (unsigned int)header.getType() << std::endl;
        return header.getSSRC();
    }
    
    //handle RTCP compound packages
    unsigned int packageLength = RTCPPackageHandler::getRTCPPackageLength(header.getLength());
    void* remainingBuffer = (char*)receiveBuffer + packageLength;
    unsigned int remainingLength = receivedSize - packageLength;
    if(RTCPPackageHandler::isRTCPPackage(remainingBuffer, remainingLength))
    {
        handleRTCPPackage(remainingBuffer, remainingLength);
    }
    return header.getSSRC();
}

void RTCPHandler::sendSourceDescription()
{
    std::cout << "RTCP: Sending Report + SDES ..." << std::endl;
    const void* buffer = isActiveSender ? createSenderReport() : createReceiverReport();
    unsigned int length = RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)buffer)->getLength());
    const void* tmp = createSourceDescription(length);
    length += RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)tmp)->getLength());
    wrapper->sendData(buffer, length);
    
    Statistics::incrementCounter(Statistics::RTCP_PACKAGES_SENT);
    Statistics::incrementCounter(Statistics::RTCP_BYTES_SENT, length);
}

void RTCPHandler::sendByePackage()
{
    std::cout << "RTCP: Sending Report + SDES + BYE ..." << std::endl;
    const void* buffer = isActiveSender ? createSenderReport() : createReceiverReport();
    unsigned int length = RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)buffer)->getLength());
    const void* tmp = createSourceDescription(length);
    length += RTCPPackageHandler::getRTCPPackageLength(((const RTCPHeader*)tmp)->getLength());
    //create bye package
    RTCPHeader byeHeader(ourselves.ssrc);
    rtcpHandler.createByePackage(byeHeader, "Program exit", length);
    length += RTCPPackageHandler::getRTCPPackageLength(byeHeader.getLength());
    wrapper->sendData(buffer, length);
    
    Statistics::incrementCounter(Statistics::RTCP_PACKAGES_SENT);
    Statistics::incrementCounter(Statistics::RTCP_BYTES_SENT, length);
    std::cout << "RTCP: BYE-Package sent." << std::endl;
}

const void* RTCPHandler::createSenderReport(unsigned int offset)
{
    RTCPHeader srHeader(ourselves.ssrc);
    
    NTPTimestamp ntpTime = NTPTimestamp::now();
    const std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    const uint32_t rtpTimestamp = ourselves.initialRTPTimestamp + now.count();
    
    SenderInformation senderReport(ntpTime, rtpTimestamp, ourselves.totalPackages, ourselves.totalBytes);
    return rtcpHandler.createSenderReportPackage(srHeader, senderReport, createReceptionReports(), offset);
}

const void* RTCPHandler::createReceiverReport(unsigned int offset)
{
    RTCPHeader rrHeader(ourselves.ssrc);
    return rtcpHandler.createReceiverReportPackage(rrHeader, createReceptionReports(), offset);
}

const std::vector<ReceptionReport> RTCPHandler::createReceptionReports()
{
    const std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    //create Reception reports for all remote participants
    const auto allParticipants = ParticipantDatabase::getAllRemoteParticipants();
    std::vector<ReceptionReport> receptionReports;
    receptionReports.reserve(allParticipants.size());
    for(auto it = allParticipants.begin(); it != allParticipants.end(); ++it)
    {
        const std::chrono::milliseconds lastSRTimestamp = (*it).second.rtcpData ? 
            std::chrono::duration_cast<std::chrono::milliseconds>((*it).second.rtcpData->lastSRTimestamp.time_since_epoch()) : std::chrono::milliseconds(0);
        ReceptionReport receptionReport;
        receptionReport.setSSRC((*it).second.ssrc);
        receptionReport.setFractionLost((*it).second.getFractionLost());
        receptionReport.setCummulativePackageLoss((*it).second.packagesLost);
        receptionReport.setExtendedHighestSequenceNumber((*it).second.extendedHighestSequenceNumber);
        receptionReport.setInterarrivalJitter((uint32_t)round((*it).second.interarrivalJitter));
        receptionReport.setLastSRTimestamp((uint32_t)lastSRTimestamp.count());
        //XXX delay since last SR /maybe last SR is wrong
        if(lastSRTimestamp.count() == 0)
            receptionReport.setDelaySinceLastSR(0);
        else
            receptionReport.setDelaySinceLastSR((now - lastSRTimestamp).count());

        if(receptionReport.getSSRC() != 0)
        {
            //we can skip initial reception-report if we don't even know for whom
            receptionReports.push_back(receptionReport);
        }
    }
    return receptionReports;
}


const void* RTCPHandler::createSourceDescription(unsigned int offset)
{
    std::vector<SourceDescription>& sourceDescriptions = ourselves.rtcpData->sourceDescriptions;
    //TODO clashes with interactive configuration (with input to shutdown server)
    if(sourceDescriptions.empty())
    {
        sourceDescriptions.push_back({RTCP_SOURCE_CNAME, (Utility::getUserName() + '@') + Utility::getDomainName()});
        if(std::dynamic_pointer_cast<InteractiveConfiguration>(configMode) == nullptr && configMode->isConfigured())
        {
            //add user configured values
            if(configMode->isCustomConfigurationSet(Parameters::SDES_EMAIL->longName, "SDES EMAIL?"))
            {
                sourceDescriptions.emplace_back(RTCP_SOURCE_EMAIL, configMode->getCustomConfiguration(Parameters::SDES_EMAIL->longName, "Enter SDES EMAIL", "anon@noreply.com"));
            }
            if(configMode->isCustomConfigurationSet(Parameters::SDES_LOC->longName, "SDES LOCATION?"))
            {
                sourceDescriptions.emplace_back(RTCP_SOURCE_LOC, configMode->getCustomConfiguration(Parameters::SDES_LOC->longName, "Enter SDES LOCATION", "earth"));
            }
            if(configMode->isCustomConfigurationSet(Parameters::SDES_NAME->longName, "SDES NAME?"))
            {
                sourceDescriptions.emplace_back(RTCP_SOURCE_NAME, configMode->getCustomConfiguration(Parameters::SDES_NAME->longName, "Enter SDES NAME", "anon"));
            }
            if(configMode->isCustomConfigurationSet(Parameters::SDES_NOTE->longName, "SDES NOTE?"))
            {
                sourceDescriptions.emplace_back(RTCP_SOURCE_NOTE, configMode->getCustomConfiguration(Parameters::SDES_NOTE->longName, "Enter SDES NOTE", ""));
            }
            if(configMode->isCustomConfigurationSet(Parameters::SDES_PHONE->longName, "SDES PHONE?"))
            {
                sourceDescriptions.emplace_back(RTCP_SOURCE_PHONE, configMode->getCustomConfiguration(Parameters::SDES_PHONE->longName, "Enter SDES PHONE", ""));
            }
        }
        sourceDescriptions.push_back({RTCP_SOURCE_TOOL, std::string("OHMComm v") + OHMCOMM_VERSION});
    }
    RTCPHeader sdesHeader(ourselves.ssrc);
    return rtcpHandler.createSourceDescriptionPackage(sdesHeader, sourceDescriptions, offset);
}

void RTCPHandler::printReceptionReports(const std::vector<ReceptionReport>& reports)
{
    if(reports.size() > 0)
    {
        std::cout << "RTCP: Received Reception Reports:" << std::endl;
        for(const ReceptionReport& report : reports)
        {
            std::cout << "\tReception Report for: " << report.getSSRC() << std::endl;
            std::cout << "\t\tExtended highest sequence number: " << report.getExtendedHighestSequenceNumber() << std::endl;
            std::cout << "\t\tFraction Lost (1/256): " << (unsigned int)report.getFractionLost() << std::endl;
            std::cout << "\t\tTotal package loss: " << report.getCummulativePackageLoss() << std::endl;
            std::cout << "\t\tInterarrival Jitter (in ms): " << report.getInterarrivalJitter() << std::endl;
        }
    }
}
