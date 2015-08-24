#ifndef TESTRTPBUFFER_H
#define TESTRTPBUFFER_H

#include "cpptest.h"
#include "RTPBufferHandler.h"
#include "RTPBuffer.h"

class TestRTPBuffer : public Test::Suite
{
public:
    TestRTPBuffer();
    ~TestRTPBuffer();

    void testMinBufferPackages();
    void testWriteFullBuffer();
    void testReadSuccessivePackages();
    void testWriteOldPackage();

private:
    const unsigned int payloadSize;
    const unsigned short maxCapacity;
    const unsigned short maxDelay;
    const unsigned short minBufferPackages;
    RTPBufferHandler* handler;
    RTPPackageHandler package;

};

#endif // TESTRTPBUFFER_H
