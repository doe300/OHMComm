#ifndef TESTNETWORKWRAPPERS_H
#define TESTNETWORKWRAPPERS_H

#include <memory>

#include "cpptest.h"
#include "network/NetworkWrapper.h"

class TestNetworkWrappers : public Test::Suite
{
public:
    TestNetworkWrappers();

    virtual ~TestNetworkWrappers();

    void testIPv4(char* type);
    void testIPv6(char* type);
    
    void testMulticastWrapper();
private:
    const unsigned int bufferSize;
    char* sendBuffer;
    char* receiveBuffer;
    
    void testWrapper(ohmcomm::network::NetworkWrapper& wrapper);
    
    static std::unique_ptr<ohmcomm::network::NetworkWrapper> getWrapper(const char* type, bool IPv6);
};

#endif // TESTNETWORKWRAPPERS_H
