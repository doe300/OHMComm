/* 
 * File:   TestCryptoSuite.h
 * Author: doe300
 *
 * Created on June 29, 2016, 4:41 PM
 */

#ifndef TESTCRYPTOSUITE_H
#define TESTCRYPTOSUITE_H

#include "cpptest.h"
#include "crypto/CryptoSuite.h"

class TestCryptoSuite : public Test::Suite
{
public:
    TestCryptoSuite();
    
    void testMD5();
    void testHMAC_SHA1();
    void testAES();

protected:
    bool setup() override;
};

#endif /* TESTCRYPTOSUITE_H */

