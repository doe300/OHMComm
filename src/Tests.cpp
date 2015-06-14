#include "Tests.h"

TestAudioIO::TestAudioIO() {
	TEST_ADD(TestAudioIO::testGetAudioIOInstances);
	TEST_ADD(TestAudioIO::testAudioProcessorInterface);
}

void TestAudioIO::testGetAudioIOInstances()
{
	// Will succeed since the expression evaluates to true
	TEST_ASSERT(1 + 1 == 2)

	// Will fail since the expression evaluates to false
	TEST_ASSERT_MSG(0 == 1, "Omg 0 ist nicht 1");

}

void TestAudioIO::testAudioProcessorInterface()
{

}