/* 
 * File:   TestNetworkGrammars.cpp
 * Author: daniel
 * 
 * Created on February 13, 2016, 1:05 PM
 */

#include "TestNetworkGrammars.h"

using namespace ohmcomm::network;

TestNetworkGrammars::TestNetworkGrammars() : Suite()
{
    TEST_ADD(TestNetworkGrammars::testToHostAndPort);
    TEST_ADD(TestNetworkGrammars::testValidHost);
    TEST_ADD(TestNetworkGrammars::testToPort);
}

void TestNetworkGrammars::testToHostAndPort()
{
    TEST_ASSERT(std::get<1>(NetworkGrammars::toHostAndPort("host.com:55")) == 55);
    TEST_ASSERT(std::get<0>(NetworkGrammars::toHostAndPort("host.com:55")).compare("host.com") == 0);
    TEST_ASSERT(std::get<1>(NetworkGrammars::toHostAndPort("host.com")) == NetworkGrammars::INVALID_PORT);
}

void TestNetworkGrammars::testValidHost()
{
    TEST_ASSERT(NetworkGrammars::isValidHost("host.com"));
    TEST_ASSERT(NetworkGrammars::isValidHost("::1"));
    TEST_ASSERT(NetworkGrammars::isValidHost("127.0.0.1"));
    TEST_ASSERT(!NetworkGrammars::isValidHost("wrong:host@null?nix"));
}

void TestNetworkGrammars::testToPort()
{
    TEST_ASSERT_EQUALS(NetworkGrammars::INVALID_PORT, NetworkGrammars::toPort("dummy"));
    TEST_ASSERT_EQUALS(100, NetworkGrammars::toPort("100"));
    TEST_ASSERT_EQUALS(NetworkGrammars::INVALID_PORT, NetworkGrammars::toPort("100000"));
}


