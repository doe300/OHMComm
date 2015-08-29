#include "TestRTPBuffer.h"

TestRTPBuffer::TestRTPBuffer() : payloadSize(511), maxCapacity(128), maxDelay(100), minBufferPackages(20),
    handler(new RTPBufferAlternative(maxCapacity, maxDelay,minBufferPackages)), package(payloadSize)
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
    for(unsigned int i = 0; i < minBufferPackages-1; i++)
    {
        void* buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
        //copies the new-buffer into the work-buffer
        package.getRTPPackageHeader(buf);
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY, handler->addPackage(package, 10));
    }
    //now we have to read a silent-package
    TEST_ASSERT(((RTP_BUFFER_OUTPUT_UNDERFLOW | RTP_BUFFER_IS_PUFFERING) & handler->readPackage(package)) != 0);
}

void TestRTPBuffer::testWriteFullBuffer()
{
    //fill whole buffer
    while(handler->getSize() < maxCapacity)
    {
        void* buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
        //copies the new-buffer into the work-buffer
        package.getRTPPackageHeader(buf);
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY,handler->addPackage(package, 10));
    }
    //write the next package -> should fail
    void* buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
    //copies the new-buffer into the work-buffer
    package.getRTPPackageHeader(buf);
    TEST_ASSERT_EQUALS(RTP_BUFFER_INPUT_OVERFLOW,handler->addPackage(package, 10));
}

void TestRTPBuffer::testReadSuccessivePackages()
{
    unsigned int seqNumber = 0;
    //read some packages and check whether their sequence numbers are successive
    for(unsigned int i = 0; i < maxCapacity / 2; i++)
    {
        handler->readPackage(package);
        if(seqNumber == 0)
        {
            seqNumber = package.getRTPPackageHeader()->sequence_number;
        }
        else
        {
            TEST_ASSERT_EQUALS(seqNumber+1, package.getRTPPackageHeader()->sequence_number);
            seqNumber = (seqNumber + 1) % UINT16_MAX;
        }
    }
}

void TestRTPBuffer::testWriteOldPackage()
{
    //write some package
    void* buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
    //copies the new-buffer into the work-buffer
    package.getRTPPackageHeader(buf);
    TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY,handler->addPackage(package, 10));
    unsigned int size = handler->getSize();

    //write a package which is far too old
    buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
    ((RTPHeader*)buf)->sequence_number -= maxCapacity;
    //copies the new-buffer into the work-buffer
    package.getRTPPackageHeader(buf);
    TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY,handler->addPackage(package, 10));
    TEST_ASSERT_EQUALS(size, handler->getSize());

}

void TestRTPBuffer::testPackageBlockLoss()
{
    if(dynamic_cast<RTPBuffer*>(handler) != nullptr)
    {
        TEST_FAIL("RTPBuffer currently does not support this test!");
        return;
    }
    if(dynamic_cast<RTPBufferAlternative*>(handler) != nullptr)
    {
        TEST_FAIL("RTPBufferAlternative currently does not support this test!");
        return;
    }
    //we first empty the buffer
    while(handler->getSize() > 0)
    {
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY, handler->readPackage(package));
    }
    //we fill a few packages
    for(int i = 0; i < 10; i++)
    {
        //write some package
        void* buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
        //copies the new-buffer into the work-buffer
        package.getRTPPackageHeader(buf);
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY,handler->addPackage(package, 10));
    }
    //skip a block (brute force)
    for(int i = 0; i < 10; i++)
    {
        //getNewRTPPackage increases sequence-number
        package.getNewRTPPackage((char*)"Dadadummi!", 10);
    }
    //fill with more packages
    for(int i = 0; i < 10; i++)
    {
        //write some package
        void* buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
        //copies the new-buffer into the work-buffer
        package.getRTPPackageHeader(buf);
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY,handler->addPackage(package, 10));
    }

    //tests - we read 30 packages
    TEST_ASSERT_EQUALS(20, handler->getSize());
    for(int i = 0; i < 10; i++)
    {
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY, handler->readPackage(package));
    }
    for(int i = 0; i < 10; i++)
    {
        TEST_ASSERT_EQUALS(RTP_BUFFER_OUTPUT_UNDERFLOW, handler->readPackage(package));
    }
    for(int i = 0; i < 10; i++)
    {
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY, handler->readPackage(package));
    }
    TEST_ASSERT_EQUALS(0, handler->getSize());
}

void TestRTPBuffer::testContinousPackageLoss()
{
    if(dynamic_cast<RTPBuffer*>(handler) != nullptr)
    {
        TEST_FAIL("RTPBuffer currently does not support this test!");
        return;
    }
    if(dynamic_cast<RTPBufferAlternative*>(handler) != nullptr)
    {
        TEST_FAIL("RTPBufferAlternative currently does not support this test!");
        return;
    }
    //we first empty the buffer
    while(handler->getSize() > 0)
    {
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY, handler->readPackage(package));
    }

    TEST_ASSERT_EQUALS(0, handler->getSize());

    unsigned int lastSeqNum;
    //we loose every second package
    for(int i = 0; i < 10; i++)
    {
        //skip this package
        package.getNewRTPPackage((char*)"Dadadummi!", 10);
        //write some package
        void* buf = package.getNewRTPPackage((char*)"Dadadummi!", 10);
        //copies the new-buffer into the work-buffer
        package.getRTPPackageHeader(buf);
        lastSeqNum = package.getRTPPackageHeader()->sequence_number;
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY,handler->addPackage(package, 10));
    }

    TEST_ASSERT_EQUALS(10, handler->getSize());
    //we need to be able to read 20 packages
    for(int i = 0; i < 10; i++)
    {
        TEST_ASSERT_EQUALS(RTP_BUFFER_OUTPUT_UNDERFLOW, handler->readPackage(package));
        TEST_ASSERT_EQUALS(RTP_BUFFER_ALL_OKAY, handler->readPackage(package));
    }
    TEST_ASSERT_EQUALS(lastSeqNum, package.getRTPPackageHeader()->sequence_number);
    TEST_ASSERT_EQUALS(0, handler->getSize());
}
