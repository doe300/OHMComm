#ifndef OHMCOMMTESTS
#define	OHMCOMMTESTS

#include "cpptest.h"
#include "AudioHandler.h"

class TestAudioIO : public Test::Suite {
	void testGetAudioIOInstances(); // getNewAudioIO(..)
	void testAudioProcessorInterface(); // add, remove, reset
public:
	TestAudioIO();
};

#endif