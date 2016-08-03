/* 
 * File:   TestCryptoSuite.cpp
 * Author: doe300
 * 
 * Created on June 29, 2016, 4:41 PM
 */

#include <string.h>

#include "TestCryptoSuite.h"
#include "Utility.h"

using namespace ohmcomm::crypto;

TestCryptoSuite::TestCryptoSuite() : Test::Suite()
{
    TEST_ADD(TestCryptoSuite::testMD5);
    TEST_ADD(TestCryptoSuite::testHMAC_SHA1);
    TEST_ADD(TestCryptoSuite::testAES);
}

bool TestCryptoSuite::setup()
{
#ifdef ENABLE_CRYPTOGRAPHICS
    return true;
#else
    return false;
#endif
}

void TestCryptoSuite::testMD5()
{
    const std::string test("Hello World!");
    const std::string hash("ed076287532e86365e841e92bfc50d8c");
    
    TEST_ASSERT(ohmcomm::Utility::equalsIgnoreCase(CryptoSuite::hashMD5(test), hash));
}

void TestCryptoSuite::testHMAC_SHA1()
{
    std::shared_ptr<CryptographicContext> context(new CryptographicContext{AES_CM_128_HMAC_SHA1_32, {'T', 'h', 'i', 's', 'i', 's', 'A', 'k', 'e', 'y'}});
    
    const std::string test("Hello World!");
    
    const auto hash = CryptoSuite::createHMAC_SHA1(test.data(), test.size(), context);
    TEST_ASSERT(CryptoSuite::verifyHMAC_SHA1(hash.data(), hash.size(), test.data(), test.size(), context));
}

void TestCryptoSuite::testAES()
{
    //AES key is 128 bit
    std::vector<byte> key(16);
    memcpy(key.data(), "0123456789ABCDEF", 16);
    
    const std::string test("Hello World!");
    
    const auto crypt = CryptoSuite::encryptAES(CipherMode::COUNTER_MODE, test.data(), test.size(), key);
    TEST_ASSERT(crypt.size() >= test.size());
    
    const auto decrypt = CryptoSuite::decryptAES(CipherMode::COUNTER_MODE, crypt.data(), crypt.size(), key);
    const std::string result((const char*)decrypt.data(), decrypt.size());
    TEST_STRING_EQUALS(test, result);
}
