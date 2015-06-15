#ifndef OHMCOMMTESTS
#define	OHMCOMMTESTS

#include "cpptest.h"
#include "AudioHandlerFactory.h"

class TestAudioIO : public Test::Suite {
	void testAudioHandlerInstances(); // getNewAudioIO(..)
	void testAudioProcessorInterface(); // add, remove, reset
public:
	TestAudioIO();
};

#endif