#include "TestNetworkWrappers.h"

using namespace ohmcomm;
using namespace ohmcomm::network;

TestNetworkWrappers::TestNetworkWrappers() : bufferSize(511), sendBuffer(new char[bufferSize]), receiveBuffer(new char[bufferSize])
{
    TEST_ADD(TestNetworkWrappers::testUDPWrapperIPv4);
    TEST_ADD(TestNetworkWrappers::testUDPWrapperIPv6);
}

TestNetworkWrappers::~TestNetworkWrappers()
{
    delete[] sendBuffer;
    delete[] receiveBuffer;
}

void TestNetworkWrappers::testUDPWrapperIPv4()
{
    UDPWrapper wrapper(DEFAULT_NETWORK_PORT, "127.0.0.1", DEFAULT_NETWORK_PORT);
    testUDPWrapper(wrapper);
}

void TestNetworkWrappers::testUDPWrapperIPv6()
{
    UDPWrapper wrapper(DEFAULT_NETWORK_PORT, "::1", DEFAULT_NETWORK_PORT);
    testUDPWrapper(wrapper);
}

void TestNetworkWrappers::testUDPWrapper(ohmcomm::network::UDPWrapper& wrapper)
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
