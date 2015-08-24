#include "TestRTPBuffer.h"

TestRTPBuffer::TestRTPBuffer() : payloadSize(511), maxCapacity(100), maxDelay(100), minBufferPackages(20),
    handler(new RTPBuffer(maxCapacity, maxDelay,minBufferPackages)), package(payloadSize)
{
    TEST_ADD(TestRTPBuffer::testMinBufferPackages);
    TEST_ADD(TestRTPBuffer::testWriteFullBuffer);
    TEST_ADD(TestRTPBuffer::testReadSuccessivePackages);
    TEST_ADD(TestRTPBuffer::testWriteOldPackage);
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
    TEST_ASSERT_EQUALS(RTP_BUFFER_OUTPUT_UNDERFLOW, handler->readPackage(package));
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
