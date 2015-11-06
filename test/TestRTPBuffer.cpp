#include "TestRTPBuffer.h"

TestRTPBuffer::TestRTPBuffer() : payloadSize(511), maxCapacity(128), maxDelay(100), minBufferPackages(20),
handler(new RTPBufferAlternative(maxCapacity, maxDelay, minBufferPackages)), package(payloadSize)
{
	TEST_ADD(TestRTPBuffer::testMinBufferPackages);
	TEST_ADD(TestRTPBuffer::testWriteFullBuffer);
	TEST_ADD(TestRTPBuffer::testReadSuccessivePackages);
	TEST_ADD(TestRTPBuffer::testWriteOldPackage);
	TEST_ADD(TestRTPBuffer::testPackageBlockLoss);
	TEST_ADD(TestRTPBuffer::testContinousPackageLoss);
}

TestRTPBuffer::~TestRTPBuffer()
{
	delete handler;
}

void TestRTPBuffer::testMinBufferPackages()
{
	//fill with less packages than minBufferSize
	for (int i = 0; i < minBufferPackages - 1; i++)
	{
		package.createNewRTPPackage((char*)"Dadadummi!", 10);
		RTPBufferStatus result = handler->addPackage(package, 10);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, result);
	}
	//now we have to read a silent-package
	RTPBufferStatus result = handler->readPackage(package);
	switch (result)
	{
	case(RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW) : break;
	case(RTPBufferStatus::RTP_BUFFER_IS_PUFFERING) : break;
	case(RTPBufferStatus::RTP_BUFFER_ALL_OKAY) : TEST_FAIL("Unexpected return value (RTP_BUFFER_ALL_OKAY) from buffer->readPackage().\n");
	default: TEST_FAIL("Unexpected return value from buffer->readPackage().\n");
	}
}

void TestRTPBuffer::testWriteFullBuffer()
{
	const char* someText = "This is some Text";
	//fill whole buffer
	while (handler->getSize() < maxCapacity)
	{
		package.createNewRTPPackage(someText, 16);
		RTPBufferStatus result = handler->addPackage(package, 10);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, result);
	}
	//write the next package -> should fail
	package.createNewRTPPackage(someText, 16);
	RTPBufferStatus result = handler->addPackage(package, 10);
	TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW, result);
}

void TestRTPBuffer::testReadSuccessivePackages()
{
	unsigned int seqNumber = 0;
	//read some packages and check whether their sequence numbers are successive
	for (unsigned int i = 0; i < maxCapacity / 2; i++)
	{
		handler->readPackage(package);
		if (seqNumber == 0)
		{
			seqNumber = package.getRTPPackageHeader()->sequence_number;
		}
		else
		{
			TEST_ASSERT_EQUALS(seqNumber + 1, package.getRTPPackageHeader()->sequence_number);
			seqNumber = (seqNumber + 1) % UINT16_MAX;
		}
	}
}

void TestRTPBuffer::testWriteOldPackage()
{
	const char* someText = "This is some Text";
	//write some package
	const void* buf = package.createNewRTPPackage(someText, 16);
	TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, handler->addPackage(package, 10));
	unsigned int size = handler->getSize();

	//write a package which is far too old
	buf = package.createNewRTPPackage(someText, 16);
	((RTPHeader*)buf)->sequence_number -= maxCapacity;

	auto result = handler->addPackage(package, 10);
	switch (result)
	{
	case(RTPBufferStatus::RTP_BUFFER_ALL_OKAY) : break;
	case(RTPBufferStatus::RTP_BUFFER_PACKAGE_TO_OLD) : break;
	default: TEST_FAIL("Unexpected return value from buffer->addPackage().\n");
	}

	auto resultSize = handler->getSize();
	TEST_ASSERT_EQUALS(size, resultSize);

}

void TestRTPBuffer::testPackageBlockLoss()
{
	if (dynamic_cast<RTPBuffer*>(handler) != nullptr)
	{
		TEST_FAIL("RTPBuffer currently does not support this test!");
		return;
	}

	//we first empty the buffer
	while (handler->getSize() > 0)
	{
		auto result = handler->readPackage(package);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, result);
	}

	//we fill a few packages
	for (int i = 0; i < 10; i++)
	{
		//write some package
		package.createNewRTPPackage((char*)"Dadadummi!", 10);
		auto result = handler->addPackage(package, 10);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, result);
	}
	//skip a block (brute force)
	for (int i = 0; i < 10; i++)
	{
		//createNewRTPPackage increases sequence-number
		package.createNewRTPPackage((char*)"Dadadummi!", 10);
	}
	//fill with more packages
	for (int i = 0; i < 10; i++)
	{
		//write some package
		package.createNewRTPPackage((char*)"Dadadummi!", 10);
		auto result = handler->addPackage(package, 10);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, result);
	}

	//tests - we read 30 packages
	TEST_ASSERT_EQUALS(20, handler->getSize());
	for (int i = 0; i < 10; i++)
	{
		auto result = handler->readPackage(package);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, result);
	}
	for (int i = 0; i < 10; i++)
	{
		auto result = handler->readPackage(package);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW, result);
	}
	for (int i = 0; i < 10; i++)
	{
		auto result = handler->readPackage(package);
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, result);
	}
	auto resultSize = handler->getSize();
	TEST_ASSERT_EQUALS(0, resultSize);
}

void TestRTPBuffer::testContinousPackageLoss()
{
	if (dynamic_cast<RTPBuffer*>(handler) != nullptr)
	{
		TEST_FAIL("RTPBuffer currently does not support this test!");
		return;
	}
	if (dynamic_cast<RTPBufferAlternative*>(handler) != nullptr)
	{
		TEST_FAIL("RTPBufferAlternative currently does not support this test!");
		return;
	}
	//we first empty the buffer
	while (handler->getSize() > 0)
	{
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, handler->readPackage(package));
	}

	TEST_ASSERT_EQUALS(0, handler->getSize());

	unsigned int lastSeqNum;
	//we loose every second package
	for (int i = 0; i < 10; i++)
	{
		//skip this package
		package.createNewRTPPackage((char*)"Dadadummi!", 10);
		//write some package
		package.createNewRTPPackage((char*)"Dadadummi!", 10);
		lastSeqNum = package.getRTPPackageHeader()->sequence_number;
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, handler->addPackage(package, 10));
	}

	TEST_ASSERT_EQUALS(10, handler->getSize());
	//we need to be able to read 20 packages
	for (int i = 0; i < 10; i++)
	{
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW, handler->readPackage(package));
		TEST_ASSERT_EQUALS(RTPBufferStatus::RTP_BUFFER_ALL_OKAY, handler->readPackage(package));
	}
	TEST_ASSERT_EQUALS(lastSeqNum, package.getRTPPackageHeader()->sequence_number);
	TEST_ASSERT_EQUALS(0, handler->getSize());
}
