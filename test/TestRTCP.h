/* 
 * File:   TestRTCP.h
 * Author: daniel
 *
 * Created on August 16, 2015, 10:49 AM
 */

#ifndef TESTRTCP_H
#define	TESTRTCP_H

#include "cpptest.h"
#include "RTCPPackageHandler.h"

class TestRTCP : public Test::Suite
{
public:
    TestRTCP();

    void testSenderReportPackage();
    
    void testReceiverReportPackage();
    
    void testSourceDescriptionPacakge();
    
    void testByePackage();
    
    void testAppDefinedPackage();
    
    void testIsRTCPPackage();
    
private:
    RTCPPackageHandler handler;
    uint32_t testSSRC = 123456789;
};

#endif	/* TESTRTCP_H */

