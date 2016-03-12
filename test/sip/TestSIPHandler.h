/* 
 * File:   TestSIPHandler.h
 * Author: daniel
 *
 * Created on November 26, 2015, 1:04 PM
 */

#ifndef TESTSIPHANDLER_H
#define	TESTSIPHANDLER_H

#include <cstdlib>
#include <chrono>
#include "cpptest.h"
#include "sip/SIPHandler.h"

class TestSIPHandler : public Test::Suite
{
public:
    TestSIPHandler();
    
    ~TestSIPHandler()
    {
    }

    void testSIPThread();
    
    void testSIPProtocol(const int index);
    
    virtual bool before(const std::string& methodName);

    virtual void after(const std::string& methodName, const bool success);

private:
    std::unique_ptr<ohmcomm::sip::SIPHandler> handler;
    
    void dummyHandler();
};

#endif	/* TESTSIPHANDLER_H */

