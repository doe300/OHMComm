/* 
 * File:   TestSocketAddress.cpp
 * Author: doe300
 * 
 * Created on June 19, 2016, 4:29 PM
 */

#include "TestSocketAddress.h"

using namespace ohmcomm::network;

TestSocketAddress::TestSocketAddress() : Test::Suite()
{
    TEST_ADD(TestSocketAddress::testAddressAndPort);
    TEST_ADD(TestSocketAddress::testLocalSocketAddress);
}

void TestSocketAddress::testAddressAndPort()
{
    const SocketAddress addr1 = SocketAddress::fromAddressAndPort("127.0.0.1", 5060);
    const auto pair = addr1.toAddressAndPort();
    TEST_ASSERT_EQUALS(pair.second, 5060);
    TEST_ASSERT(pair.first.compare("127.0.0.1") == 0);
}

void TestSocketAddress::testLocalSocketAddress()
{
    const SocketAddress addr1 = SocketAddress::createLocalAddress(false, 5060);
    const auto pair = addr1.toAddressAndPort();
    TEST_ASSERT_EQUALS(pair.second, 5060);
}

