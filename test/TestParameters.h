/* 
 * File:   TestParameters.h
 * Author: daniel
 *
 * Created on July 24, 2015, 1:41 PM
 */

#ifndef TESTPARAMETERS_H
#define	TESTPARAMETERS_H

#include "cpptest.h"
#include "Parameters.h"

class TestParameters : public Test::Suite
{
public:
    TestParameters();

    void testParseParameters();
    
    void testIsParameterSet();
    
    void testGetParameterValue();
    
    void testGetAudioProcessors();
    
private:
    Parameters params;
};

#endif	/* TESTPARAMETERS_H */

