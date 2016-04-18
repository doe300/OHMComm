/* 
 * File:   TestSDP.h
 * Author: doe300
 *
 * Created on April 18, 2016, 5:19 PM
 */

#ifndef TESTSDP_H
#define TESTSDP_H

#include "cpptest.h"

#include "sip/SDPMessageHandler.h"

class TestSDP : public Test::Suite
{
public:
    TestSDP();
    
    void testSessionDescription();
    void testMediaDescription();
};

#endif /* TESTSDP_H */

