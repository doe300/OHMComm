#ifndef TESTRTPBUFFER_H
#define TESTRTPBUFFER_H

#include "cpptest.h"
#include "rtp/RTPBufferHandler.h"
#include "rtp/RTPBuffer.h"
#include "rtp/RTPBufferAlternative.h"

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
    ohmcomm::rtp::RTPBufferHandler* handler;
    ohmcomm::rtp::RTPPackageHandler package;

};

#endif // TESTRTPBUFFER_H
