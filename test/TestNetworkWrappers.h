#ifndef TESTNETWORKWRAPPERS_H
#define TESTNETWORKWRAPPERS_H

#include "cpptest.h"
#include "network/NetworkWrapper.h"
#include "network/UDPWrapper.h"

class TestNetworkWrappers : public Test::Suite
{
public:
    TestNetworkWrappers();

    virtual ~TestNetworkWrappers();

    void testUDPWrapperIPv4();
    void testUDPWrapperIPv6();
private:
    const unsigned int bufferSize;
    char* sendBuffer;
    char* receiveBuffer;
    
    void testUDPWrapper(UDPWrapper& wrapper);
};

#endif // TESTNETWORKWRAPPERS_H
