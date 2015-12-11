/* 
 * File:   TestSIPHandler.h
 * Author: daniel
 *
 * Created on November 26, 2015, 1:04 PM
 */

#ifndef TESTSIPHANDLER_H
#define	TESTSIPHANDLER_H

#include <chrono>
#include "cpptest.h"
#include "sip/SIPHandler.h"

class TestSIPHandler : public Test::Suite
{
public:
    TestSIPHandler();

    void testSIPThread();
    
private:
    SIPHandler handler;
};

#endif	/* TESTSIPHANDLER_H */

