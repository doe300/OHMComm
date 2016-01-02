/* 
 * File:   TestSTUNClient.cpp
 * Author: daniel
 * 
 * Created on January 2, 2016, 2:00 PM
 */

#include "TestSTUNClient.h"
#include "sip/STUNClient.h"

TestSTUNClient::TestSTUNClient()
{
    TEST_ADD(TestSTUNClient::testSTUNRequest);
}

void TestSTUNClient::testSTUNRequest()
{
    STUNClient cl;
    auto result = cl.retrieveSIPInfo();
    TEST_ASSERT(std::get<2>(result) != 0);
}
