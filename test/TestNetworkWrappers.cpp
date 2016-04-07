#include "TestNetworkWrappers.h"

#include "network/UDPWrapper.h"
#include "network/TCPWrapper.h"
#include "network/MulticastNetworkWrapper.h"

using namespace ohmcomm;
using namespace ohmcomm::network;

static const char* UDP = "UDP";
static const char* TCP = "TCP";
static const char* MULTICAST = "MULTICAST";

TestNetworkWrappers::TestNetworkWrappers() : bufferSize(511), sendBuffer(new char[bufferSize]), receiveBuffer(new char[bufferSize])
{
    TEST_ADD_WITH_STRING_LITERAL(TestNetworkWrappers::testIPv4, (char*)UDP);
    TEST_ADD_WITH_STRING_LITERAL(TestNetworkWrappers::testIPv4, (char*)TCP);
    TEST_ADD_WITH_STRING_LITERAL(TestNetworkWrappers::testIPv4, (char*)MULTICAST);
    TEST_ADD_WITH_STRING_LITERAL(TestNetworkWrappers::testIPv6, (char*)UDP);
    TEST_ADD_WITH_STRING_LITERAL(TestNetworkWrappers::testIPv6, (char*)TCP);
    TEST_ADD_WITH_STRING_LITERAL(TestNetworkWrappers::testIPv6, (char*)MULTICAST);
    TEST_ADD(TestNetworkWrappers::testMulticastWrapper);
}

TestNetworkWrappers::~TestNetworkWrappers()
{
    delete[] sendBuffer;
    delete[] receiveBuffer;
}

void TestNetworkWrappers::testIPv4(char* type)
{
    auto ptr = getWrapper(type, false);
    testWrapper(*ptr);
}

void TestNetworkWrappers::testIPv6(char* type)
{
    auto ptr = getWrapper(type, true);
    testWrapper(*ptr);
}

void TestNetworkWrappers::testMulticastWrapper()
{
    ohmcomm::network::MulticastNetworkWrapper sender({DEFAULT_NETWORK_PORT, "127.0.0.1", DEFAULT_NETWORK_PORT + 1});
    TEST_ASSERT(sender.addDestination("127.0.0.1", DEFAULT_NETWORK_PORT + 2));
    ohmcomm::network::UDPWrapper receiver1(ohmcomm::NetworkConfiguration{DEFAULT_NETWORK_PORT + 1, "127.0.0.1", DEFAULT_NETWORK_PORT});
    ohmcomm::network::UDPWrapper receiver2(ohmcomm::NetworkConfiguration{DEFAULT_NETWORK_PORT + 2, "127.0.0.1", DEFAULT_NETWORK_PORT});
    
    const char* sendBuffer = "Hello, I will be sent and stuff!";
    const int sendLength = strlen(sendBuffer);
    
    TEST_ASSERT_EQUALS(sendLength, sender.sendData(sendBuffer, sendLength));
    
    char recvBuffer1[sendLength + 10];
    char recvBuffer2[sendLength + 10];
    
    int receiveSize = -2;
    //repeat on timeout
    while(receiveSize == -2)
    {
        receiveSize = receiver1.receiveData(recvBuffer1, sendLength + 5).status;
    }
    //make sure, both recipients receive complete package
    TEST_ASSERT_EQUALS(sendLength, receiveSize);
    
    receiveSize = -2;
    //repeat on timeout
    while(receiveSize == -2)
    {
        receiveSize = receiver2.receiveData(recvBuffer2, sendLength + 5).status;
    }
    TEST_ASSERT_EQUALS(sendLength, receiveSize);
    
    //test contents for equality
    TEST_ASSERT(strcmp(sendBuffer, recvBuffer1) == 0);
    TEST_ASSERT(strcmp(sendBuffer, recvBuffer2) == 0);
    
    
    //remove recipient
    TEST_ASSERT(sender.removeDestination("127.0.0.1", DEFAULT_NETWORK_PORT + 2));
    
    //test again
    TEST_ASSERT_EQUALS(sendLength, sender.sendData(sendBuffer, sendLength));
    receiveSize = -2;
    //repeat on timeout
    while(receiveSize == -2)
    {
        receiveSize = receiver1.receiveData(recvBuffer1, sendLength + 5).status;
    }
    TEST_ASSERT_EQUALS(sendLength, receiveSize);
}

void TestNetworkWrappers::testWrapper(ohmcomm::network::NetworkWrapper& wrapper)
{
    const char* text = "This is a test, Lorem ipsum! We fill this buffer with some random stuff ..... And send a arbitrary amount of bytes and compare them to this original string...";
    strncpy(sendBuffer, text, 150);

    //single package test
    int sendBytes = wrapper.sendData(sendBuffer, 150);
    TEST_ASSERT_MSG(sendBytes > 0, "Error sending UDP package");
    if(sendBytes <= 0)
    {
        std::wcerr << wrapper.getLastError() << std::endl;
    }

    int receivedBytes = wrapper.receiveData(receiveBuffer, bufferSize).status;
    TEST_ASSERT_EQUALS_MSG(sendBytes, receivedBytes, "Package sizes do no match!");
    if(receivedBytes <= 0)
    {
        std::wcerr << wrapper.getLastError() << std::endl;
    }

    TEST_ASSERT_EQUALS(std::string(sendBuffer, sendBytes), std::string(receiveBuffer, receivedBytes));

    //test with sending several packages before receiving
    for(unsigned int i = 1; i < 5; i++)
    {
        int sendBytes = wrapper.sendData(sendBuffer, i*30);
        TEST_ASSERT_MSG(sendBytes > 0, "Error sending UDP package");
        if(sendBytes <= 0)
        {
            std::wcerr << wrapper.getLastError() << std::endl;
        }
    }
    for(int i = 1; i < 5; i++)
    {
        int receivedBytes = wrapper.receiveData(receiveBuffer, bufferSize).status;
        TEST_ASSERT_EQUALS_MSG(i*30, receivedBytes, "Package sizes do no match!");
        if(receivedBytes <= 0)
        {
            std::wcerr << wrapper.getLastError() << std::endl;
        }
    }


    wrapper.closeNetwork();
}

std::unique_ptr<ohmcomm::network::NetworkWrapper> TestNetworkWrappers::getWrapper(const char* type, bool IPv6)
{
    const std::string ip = IPv6 ? "::1" : "127.0.0.1";
    if(type == UDP)
    {
        return std::unique_ptr<ohmcomm::network::NetworkWrapper>(new ohmcomm::network::UDPWrapper(ohmcomm::NetworkConfiguration{DEFAULT_NETWORK_PORT, ip, DEFAULT_NETWORK_PORT}));
    }
    if(type == TCP)
    {
        return std::unique_ptr<ohmcomm::network::NetworkWrapper>(new ohmcomm::network::TCPWrapper(ohmcomm::NetworkConfiguration{DEFAULT_NETWORK_PORT, ip, DEFAULT_NETWORK_PORT}));
    }
    return std::unique_ptr<ohmcomm::network::NetworkWrapper>(new ohmcomm::network::MulticastNetworkWrapper(ohmcomm::NetworkConfiguration{DEFAULT_NETWORK_PORT, ip, DEFAULT_NETWORK_PORT}));
}
