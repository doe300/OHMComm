#ifndef TESTRTPBUFFER_H
#define TESTRTPBUFFER_H

#include "cpptest.h"
#include "RTPBufferHandler.h"
#include "RTPBuffer.h"
#include "RTPBufferAlternative.h"

class TestRTPBuffer : public Test::Suite
{
public:
    TestRTPBuffer();
    ~TestRTPBuffer();

    void testMinBufferPackages();
    void testWriteFullBuffer();
    void testReadSuccessivePackages();
    void testWriteOldPackage();
    void testPackageBlockLoss();
    void testContinousPackageLoss();

private:
    const unsigned int payloadSize;
    const unsigned short maxCapacity;
    const unsigned short maxDelay;
    const unsigned short minBufferPackages;
    RTPBufferHandler* handler;
    RTPPackageHandler package;

};

#endif // TESTRTPBUFFER_H
