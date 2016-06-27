/* 
 * File:   TestSIPRequest.cpp
 * Author: doe300
 * 
 * Created on June 27, 2016, 2:23 PM
 */

#include "TestSIPRequest.h"
#include "network/UDPWrapper.h"

using namespace ohmcomm::sip;

#define READ_REQUEST \
{ \
    package = waitForPackage(); \
    TEST_ASSERT(SIPPackageHandler::isRequestPackage(buffer.data(), package.getReceivedSize())); \
    body = SIPPackageHandler::readRequestPackage(buffer.data(), package.getReceivedSize(), requestHeader); \
    TEST_THROWS_NOTHING(SIPPackageHandler::checkSIPHeader(&requestHeader)); \
}

#define READ_RESPONSE \
{ \
    package = waitForPackage(); \
    TEST_ASSERT(SIPPackageHandler::isResponsePackage(buffer.data(), package.getReceivedSize())); \
    body = SIPPackageHandler::readResponsePackage(buffer.data(), package.getReceivedSize(), responseHeader); \
    TEST_THROWS_NOTHING(SIPPackageHandler::checkSIPHeader(&responseHeader)); \
}

TestSIPRequest::TestSIPRequest(): Test::Suite(""), network(new ohmcomm::network::UDPWrapper(5060, "127.0.0.1", 5060)), 
        buffer(4096), thisUA(std::to_string(rand())), remoteUA(std::to_string(rand()))
{
    TEST_ADD(TestSIPRequest::testRegisterRequest);
    TEST_ADD(TestSIPRequest::testOptionsRequest);
    TEST_ADD(TestSIPRequest::testInfoRequest);
    TEST_ADD(TestSIPRequest::testCancelRequest);
    TEST_ADD(TestSIPRequest::testByeRequest);
    TEST_ADD(TestSIPRequest::testInviteRequest);
    
    thisUA.callID = "testCall";
    thisUA.hostName = "localhost";
    thisUA.ipAddress = "127.0.0.1";
    thisUA.port = 5060;
    thisUA.userName = "this";
    
    remoteUA.callID = "testCall";
    remoteUA.hostName = "localhost";
    remoteUA.ipAddress = "127.0.0.1";
    remoteUA.port = 5060;
    remoteUA.userName = "remote";
}

void TestSIPRequest::testRegisterRequest()
{
    REGISTERRequest req(thisUA, remoteUA, thisUA.port, network.get(), "testUser", "testPass");
    TEST_ASSERT(req.sendRequest(""));
    
    SIPRequestHeader requestHeader;
    ohmcomm::network::NetworkWrapper::Package package;
    std::string body;
    READ_REQUEST;
    
    TEST_STRING_EQUALS(SIP_REQUEST_OPTIONS, requestHeader.requestCommand);
    
    {
        REGISTERRequest res(thisUA, remoteUA, thisUA.port, network.get(), "testUser", "testPass");
        //handles request and sends response (NOT IMPLEMENTED)
        TEST_ASSERT(res.handleRequest(body));
        TEST_ASSERT(res.isCompleted());
    }
    
    SIPResponseHeader responseHeader;
    READ_RESPONSE;
    
    //handles response
    TEST_ASSERT(req.handleResponse(package.address, responseHeader, body, remoteUA));
    
    TEST_ASSERT_EQUALS(SIP_RESPONSE_NOT_IMPLEMENTED_CODE, responseHeader.statusCode);
    TEST_ASSERT(!req.isCompleted());
}

void TestSIPRequest::testOptionsRequest()
{
    OPTIONSRequest req(thisUA, {}, remoteUA, thisUA.port, network.get());
    TEST_ASSERT(req.sendRequest(""));
    
    SIPRequestHeader requestHeader;
    ohmcomm::network::NetworkWrapper::Package package;
    std::string body;
    READ_REQUEST;
    
    TEST_STRING_EQUALS(SIP_REQUEST_OPTIONS, requestHeader.requestCommand);
    
    {
        OPTIONSRequest res(thisUA, requestHeader, remoteUA, thisUA.port, network.get());
        //handles request and sends response (200 OK with additional info)
        TEST_ASSERT(res.handleRequest(body));
        TEST_ASSERT(res.isCompleted());
    }
    
    SIPResponseHeader responseHeader;
    READ_RESPONSE;
    
    //handles response
    TEST_ASSERT(req.handleResponse(package.address, responseHeader, body, remoteUA));
    
    TEST_ASSERT_EQUALS(SIP_RESPONSE_OK_CODE, responseHeader.statusCode);
    TEST_ASSERT(!responseHeader[SIP_HEADER_ACCEPT].empty());
    TEST_ASSERT(!responseHeader[SIP_HEADER_ALLOW].empty());
    TEST_ASSERT(req.isCompleted());
}

void TestSIPRequest::testInfoRequest()
{
    INFORequest req(thisUA, {}, remoteUA, thisUA.port, network.get());
    TEST_ASSERT(req.sendRequest(""));
    
    SIPRequestHeader requestHeader;
    ohmcomm::network::NetworkWrapper::Package package;
    std::string body;
    READ_REQUEST;
    
    TEST_STRING_EQUALS(SIP_REQUEST_INFO, requestHeader.requestCommand);
    
    {
        INFORequest res(thisUA, requestHeader, remoteUA, thisUA.port, network.get());
        //handles request and sends response (200 OK)
        TEST_ASSERT(res.handleRequest(body));
        TEST_ASSERT(res.isCompleted());
    }
    
    SIPResponseHeader responseHeader;
    READ_RESPONSE;
    
    //handles response
    TEST_ASSERT(req.handleResponse(package.address, responseHeader, body, remoteUA));
    
    TEST_ASSERT_EQUALS(SIP_RESPONSE_OK_CODE, responseHeader.statusCode);
    TEST_ASSERT(req.isCompleted());
}

void TestSIPRequest::testCancelRequest()
{
    CANCELRequest req(thisUA, {}, remoteUA, thisUA.port, network.get());
    TEST_ASSERT(req.sendRequest(""));
    
    SIPRequestHeader requestHeader;
    ohmcomm::network::NetworkWrapper::Package package;
    std::string body;
    READ_REQUEST;
    
    TEST_STRING_EQUALS(SIP_REQUEST_CANCEL, requestHeader.requestCommand);
    
    {
        CANCELRequest res(thisUA, requestHeader, remoteUA, thisUA.port, network.get());
        //handles request and sends response (200 OK)
        TEST_ASSERT(res.handleRequest(body));
        TEST_ASSERT(res.isCompleted());
    }
    
    SIPResponseHeader responseHeader;
    READ_RESPONSE;
    
    //handles response
    TEST_ASSERT(req.handleResponse(package.address, responseHeader, body, remoteUA));
    
    TEST_ASSERT_EQUALS(SIP_RESPONSE_OK_CODE, responseHeader.statusCode);
    TEST_ASSERT(req.isCompleted());
}

void TestSIPRequest::testByeRequest()
{
    const std::string message = "Dummy";
    BYERequest req(thisUA, {}, remoteUA, thisUA.port, network.get());
    TEST_ASSERT(req.sendRequest(message));
    
    SIPRequestHeader requestHeader;
    ohmcomm::network::NetworkWrapper::Package package;
    std::string body;
    READ_REQUEST;
    
    TEST_STRING_EQUALS(SIP_REQUEST_BYE, requestHeader.requestCommand);
    TEST_STRING_EQUALS(message, body);
    
    {
        BYERequest res(thisUA, requestHeader, remoteUA, thisUA.port, network.get());
        //handles request and sends response (200 OK)
        TEST_ASSERT(res.handleRequest(body));
        TEST_ASSERT(res.isCompleted());
    }
    
    SIPResponseHeader responseHeader;
    READ_RESPONSE;
    
    //handles response
    TEST_ASSERT(req.handleResponse(package.address, responseHeader, body, remoteUA));
    
    TEST_ASSERT_EQUALS(SIP_RESPONSE_OK_CODE, responseHeader.statusCode);
    TEST_ASSERT(req.isCompleted());
}

void TestSIPRequest::testInviteRequest()
{
    const INVITERequest::ConnectCallback call = [this](const MediaDescription& descr, const ohmcomm::NetworkConfiguration& rtpConfig, const ohmcomm::NetworkConfiguration rtcpConfig) -> void
    {
        TEST_STRING_EQUALS("opus", descr.encoding);
        TEST_ASSERT_EQUALS(2, descr.numChannels);
        TEST_ASSERT_EQUALS(ohmcomm::PayloadType::OPUS, descr.payloadType);
        TEST_ASSERT_EQUALS(SessionDescription::SDP_MEDIA_RTP, descr.protocol);
        
        TEST_ASSERT_EQUALS(ohmcomm::DEFAULT_NETWORK_PORT, rtpConfig.localPort);
    };
    
    INVITERequest req(thisUA, {}, remoteUA, thisUA.port, network.get(), SIPSession::SessionState::DISCONNECTED, call);
    TEST_ASSERT(req.sendRequest(SDPMessageHandler::createSessionDescription(thisUA.userName, {ohmcomm::DEFAULT_NETWORK_PORT, "127.0.0.1", ohmcomm::DEFAULT_NETWORK_PORT})));
    
    SIPRequestHeader requestHeader;
    ohmcomm::network::NetworkWrapper::Package package;
    std::string body;
    READ_REQUEST;
    
    TEST_STRING_EQUALS(SIP_REQUEST_INVITE, requestHeader.requestCommand);
    TEST_STRING_EQUALS(MIME_SDP, requestHeader[SIP_HEADER_CONTENT_TYPE]);
    
    {
        INVITERequest res(thisUA, requestHeader, remoteUA, thisUA.port, network.get(), SIPSession::SessionState::DISCONNECTED, call);
        //handles request and sends response (200 OK with additional info)
        TEST_ASSERT(res.handleRequest(body));
        TEST_ASSERT(res.isCompleted());
    }
    
    SIPResponseHeader responseHeader;
    //skip "Ringing" and "Dialog Established"
    READ_RESPONSE;
    READ_RESPONSE;
    READ_RESPONSE;
    
    //handles response
    TEST_ASSERT(req.handleResponse(package.address, responseHeader, body, remoteUA));
    
    TEST_ASSERT_EQUALS(SIP_RESPONSE_OK_CODE, responseHeader.statusCode);
    TEST_STRING_EQUALS(MIME_SDP, responseHeader[SIP_HEADER_CONTENT_TYPE]);
    TEST_ASSERT(req.isCompleted());
}

ohmcomm::network::NetworkWrapper::Package TestSIPRequest::waitForPackage()
{
    ohmcomm::network::NetworkWrapper::Package result = network->receiveData(buffer.data(), buffer.size());
    while (result.hasTimedOut())
    {
        result = network->receiveData(buffer.data(), buffer.size());
    }
    return result;
}