#include "TestNetworkWrappers.h"

TestNetworkWrappers::TestNetworkWrappers() : bufferSize(511), sendBuffer(new char[bufferSize]), receiveBuffer(new char[bufferSize])
{
    TEST_ADD(TestNetworkWrappers::testUDPWrapper);
}

TestNetworkWrappers::~TestNetworkWrappers()
{
    delete[] sendBuffer;
    delete[] receiveBuffer;
}

void TestNetworkWrappers::testUDPWrapper()
{
    UDPWrapper wrapper(DEFAULT_NETWORK_PORT, "127.0.0.1", DEFAULT_NETWORK_PORT);
    *sendBuffer = *"This is a test, Lorem ipsum! We fill this buffer with some random stuff ..... And send a arbitrary amount of bytes and compare them to this original string...";

    int sendBytes = wrapper.sendData(sendBuffer, 150);
    TEST_ASSERT_MSG(sendBytes > 0, "Error sending UDP package");
    if(sendBytes <= 0)
    {
        std::wcerr << wrapper.getLastError() << std::endl;
    }

    int receivedBytes = wrapper.receiveData(receiveBuffer, bufferSize);
    TEST_ASSERT_EQUALS_MSG(sendBytes, receivedBytes, "Package sizes do no match!");
    if(receivedBytes <= 0)
    {
        std::wcerr << wrapper.getLastError() << std::endl;
    }

    TEST_ASSERT_EQUALS(std::string(sendBuffer, sendBytes), std::string(receiveBuffer, receivedBytes));

    wrapper.closeNetwork();
}
