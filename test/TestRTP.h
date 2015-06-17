/* 
 * File:   TestRTP.h
 * Author: daniel
 *
 * Created on June 16, 2015, 8:26 PM
 */

#ifndef TESTRTP_H
#define	TESTRTP_H

#include "cpptest.h"
#include "RTCPPackageHandler.h"
#include "RTPPackageHandler.h"

class TestRTP: public Test::Suite
{
public:
    TestRTP();
    
    void testRTCPByeMessage();
    
    void testRTPPackage();
};

#endif	/* TESTRTP_H */

