/* 
 * File:   TestAudioHandler.h
 * Author: doe300
 *
 * Created on April 18, 2016, 4:55 PM
 */

#ifndef TESTAUDIOHANDLER_H
#define TESTAUDIOHANDLER_H

#include "cpptest.h"

#include "audio/AudioHandlerFactory.h"

class TestAudioHandler : public Test::Suite {
    void testAudioHandlerInstances(const std::string processorName); // getNewAudioIO(..)
    void testAudioProcessorInterface(); // add, remove, reset
    void testAudioDevices(const std::string processorName);
public:
    TestAudioHandler();
};

#endif /* TESTAUDIOHANDLER_H */

