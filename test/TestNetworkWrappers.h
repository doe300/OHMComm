#ifndef TESTNETWORKWRAPPERS_H
#define TESTNETWORKWRAPPERS_H

#include "cpptest.h"
#include "NetworkWrapper.h"
#include "UDPWrapper.h"

class TestNetworkWrappers : public Test::Suite
{
public:
    TestNetworkWrappers();

    virtual ~TestNetworkWrappers();

    void testUDPWrapper();
private:
    const unsigned int bufferSize;
    char* sendBuffer;
    char* receiveBuffer;
};

#endif // TESTNETWORKWRAPPERS_H
