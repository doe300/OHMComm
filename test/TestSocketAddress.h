/* 
 * File:   TestSocketAddress.h
 * Author: doe300
 *
 * Created on June 19, 2016, 4:29 PM
 */

#ifndef TESTSOCKETADDRESS_H
#define TESTSOCKETADDRESS_H

#include "cpptest.h"
#include "network/SocketAddress.h"

class TestSocketAddress : public Test::Suite
{
public:
    TestSocketAddress();

    void testAddressAndPort();
    
    void testLocalSocketAddress();
};

#endif /* TESTSOCKETADDRESS_H */

