/* 
 * File:   TestSIPPackages.cpp
 * Author: doe300
 * 
 * Created on April 18, 2016, 5:19 PM
 */

#include "TestSIPPackages.h"

#include "Utility.h"

using namespace ohmcomm::sip;

TestSIPPackages::TestSIPPackages()
{
    TEST_ADD(TestSIPPackages::testRequestPackage);
    TEST_ADD(TestSIPPackages::testResponsePackage);
    TEST_ADD(TestSIPPackages::testMultipartBody);
}

void TestSIPPackages::testRequestPackage()
{
    const std::string body = "Sometext";
    SIPRequestHeader input(SIP_REQUEST_INVITE, "sip:user@localhost");
    input[SIP_HEADER_USER_AGENT] = "DummyAgent";
    const std::string requestPackage = SIPPackageHandler::createRequestPackage(input, body);
    
    TEST_ASSERT(requestPackage.size() > 0);
    TEST_ASSERT(SIPPackageHandler::isRequestPackage(requestPackage.data(), requestPackage.size()));
    
    SIPRequestHeader output;
    const std::string outputBody = SIPPackageHandler::readRequestPackage(requestPackage.data(), requestPackage.size(), output);
    
    TEST_ASSERT_EQUALS(input.getRequestCommand(), output.getRequestCommand());
    TEST_ASSERT_EQUALS(input.getFieldValues(SIP_HEADER_USER_AGENT).front(), output.getFieldValues(SIP_HEADER_USER_AGENT).front());
    TEST_ASSERT(outputBody.compare(body) == 0);
}

void TestSIPPackages::testResponsePackage()
{
    const std::string body = "Sometext";
    SIPResponseHeader input(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
    input[SIP_HEADER_USER_AGENT] = "DummyAgenz";
    const std::string responsePackage = SIPPackageHandler::createResponsePackage(input, body);
    
    TEST_ASSERT(responsePackage.size() > 0);
    TEST_ASSERT(SIPPackageHandler::isResponsePackage(responsePackage.data(), responsePackage.size()));
    
    SIPResponseHeader output;
    const std::string outputBody = SIPPackageHandler::readResponsePackage(responsePackage.data(), responsePackage.size(), output);
    
    TEST_ASSERT_EQUALS(input.statusCode, output.statusCode);
    TEST_ASSERT_EQUALS(input.getFieldValues(SIP_HEADER_USER_AGENT).front(), output.getFieldValues(SIP_HEADER_USER_AGENT).front());
    TEST_ASSERT(outputBody.compare(body) == 0);
}

void TestSIPPackages::testMultipartBody()
{
    const std::string body = "--XXX\nSometext\n--XXX\nContent-type: text/html\nOthertext\n--XXX--";
    SIPResponseHeader input(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
    input[SIP_HEADER_CONTENT_TYPE] = MIME_MULTIPART_MIXED + ";boundary=XXX";
    const std::string responsePackage = SIPPackageHandler::createResponsePackage(input, body);
    
    TEST_ASSERT(SIPPackageHandler::hasMultipartBody(input));
    
    SIPResponseHeader output;
    const std::string outputBody = SIPPackageHandler::readResponsePackage(responsePackage.data(), responsePackage.size(), output);
    
    auto multipart = SIPPackageHandler::readMultipartBody(output, outputBody);
    TEST_ASSERT_EQUALS(2, multipart.size());
    
    TEST_ASSERT_EQUALS(0, ohmcomm::Utility::trim(multipart["text/plain"]).compare("Sometext"));
    TEST_ASSERT_EQUALS(0, ohmcomm::Utility::trim(multipart["text/html"]).compare("Othertext"));
}


