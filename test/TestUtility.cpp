/* 
 * File:   TestUtility.cpp
 * Author: daniel
 * 
 * Created on December 9, 2015, 4:28 PM
 */

#include "TestUtility.h"

TestUtility::TestUtility()
{
    TEST_ADD(TestUtility::printDomainInfo);
    TEST_ADD(TestUtility::testNetworkInfo);
    TEST_ADD(TestUtility::testAddressFromHostName);
    TEST_ADD(TestUtility::testTrim);
    TEST_ADD(TestUtility::testEqualsIgnoreCase);
}

void TestUtility::printDomainInfo()
{
    std::cout << "Domain name: " << Utility::getDomainName() << std::endl
            << "User name: " << Utility::getUserName() << std::endl
            << "Loopback address: " << Utility::getLocalIPAddress(Utility::AddressType::ADDRESS_LOOPBACK) << std::endl
            << "Local address: " << Utility::getLocalIPAddress(Utility::AddressType::ADDRESS_LOCAL_NETWORK) << std::endl
            << "External address: " << Utility::getLocalIPAddress(Utility::AddressType::ADDRESS_INTERNET) << std::endl;
}

void TestUtility::testNetworkInfo()
{
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_LOOPBACK, Utility::getNetworkType("127.0.0.1"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_LOOPBACK, Utility::getNetworkType("::1"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_LOCAL_NETWORK, Utility::getNetworkType("10.11.12.13"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_LOCAL_NETWORK, Utility::getNetworkType("172.24.25.26"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_LOCAL_NETWORK, Utility::getNetworkType("192.168.2.13"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_LOCAL_NETWORK, Utility::getNetworkType("fd00::d250:99ff:fe5e:4907"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_INTERNET, Utility::getNetworkType("81.81.81.81"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_INTERNET, Utility::getNetworkType("7.7.7.7"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_INTERNET, Utility::getNetworkType("2002:ADC2:712F:0:0:0:0:0"));
    TEST_ASSERT_EQUALS(Utility::AddressType::ADDRESS_INTERNET, Utility::getNetworkType("::ADC2:712F"));
}

void TestUtility::testAddressFromHostName()
{
    TEST_ASSERT(Utility::getAddressForHostName("checkip.dyndns.org").compare("91.198.22.70") == 0);
}

void TestUtility::testTrim()
{
    TEST_ASSERT_EQUALS(std::string("trimmed text"), Utility::trim("  trimmed text   "));
}

void TestUtility::testEqualsIgnoreCase()
{
    TEST_ASSERT(Utility::equalsIgnoreCase(std::string("some Text"), std::string("SoMe tExT")));
}


