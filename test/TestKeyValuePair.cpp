/* 
 * File:   TestKeyValuePair.cpp
 * Author: doe300
 * 
 * Created on June 19, 2016, 4:39 PM
 */

#include "TestKeyValuePair.h"

using namespace ohmcomm;

TestKeyValuePair::TestKeyValuePair()
{
    TEST_ADD(TestKeyValuePair::testKeyValuePair);
    TEST_ADD(TestKeyValuePair::testKeyValuePairs);
}

void TestKeyValuePair::testKeyValuePair()
{
    const PairType pair("key", "  100  ");
    TEST_ASSERT_EQUALS(100, pair.getInteger());
    TEST_ASSERT_EQUALS(1, pair.getValues().size());
    TEST_ASSERT(pair.getValues()[0].compare("100") == 0);
    
    PairType pair2;
    TEST_ASSERT(pair2.key.empty());
    TEST_ASSERT(pair2.value.empty());
    pair2.fromString("  key = value  ", '=');
    TEST_ASSERT(pair2.key.compare("key") == 0);
    TEST_ASSERT(pair2.value.compare("value") == 0);
}

void TestKeyValuePair::testKeyValuePairs()
{
    MapType map;
    map["key"] = "100";
    TEST_ASSERT(map["key"].compare("100") == 0);
    TEST_ASSERT(map.hasKey("key"));
    TEST_ASSERT(!map.hasKey("nokey"));
    map["multiple"] = "100, 200, 300, 400, 500";
    TEST_ASSERT_EQUALS(5, map.getFieldValues("multiple").size());
    TEST_ASSERT(map.getFieldValues("multiple")[4].compare("500") == 0);
    map["key"] = "200";
    TEST_ASSERT(map["key"].compare("200") == 0);
}
