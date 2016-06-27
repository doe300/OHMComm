/* 
 * File:   TestSIPRequest.h
 * Author: doe300
 *
 * Created on June 27, 2016, 2:23 PM
 */

#ifndef TESTSIPREQUEST_H
#define TESTSIPREQUEST_H

#include <utility>
#include <memory>

#include "cpptest.h"
#include "sip/SIPRequest.h"

class TestSIPRequest : public Test::Suite
{
public:
    TestSIPRequest();

    void testRegisterRequest();
    void testOptionsRequest();
    void testInfoRequest();
    void testByeRequest();
    void testCancelRequest();
    void testInviteRequest();

private:
    std::unique_ptr<ohmcomm::network::NetworkWrapper> network;
    std::vector<char> buffer;
    ohmcomm::sip::SIPUserAgent thisUA;
    ohmcomm::sip::SIPUserAgent remoteUA;
    
    ohmcomm::network::NetworkWrapper::Package waitForPackage();
};

#endif /* TESTSIPREQUEST_H */

