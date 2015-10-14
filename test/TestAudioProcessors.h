/* 
 * File:   TestAudioProcessors.h
 * Author: daniel
 *
 * Created on September 23, 2015, 5:26 PM
 */

#ifndef TESTAUDIOPROCESSORS_H
#define	TESTAUDIOPROCESSORS_H

#include <vector>

#include "cpptest.h"
#include "AudioProcessorFactory.h"

class TestAudioProcessors : public Test::Suite
{
public:
    TestAudioProcessors();

    void testAudioProcessorConfiguration(const std::string processorName);
    
private:
    std::vector<unsigned int> getSampleRates(unsigned int supportedRatesFlag);

};

#endif	/* TESTAUDIOPROCESSORS_H */

