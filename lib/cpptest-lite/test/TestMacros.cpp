/* 
 * File:   TestMacros.cpp
 * Author: daniel
 * 
 * Created on September 15, 2015, 6:27 PM
 */

#include <string.h>

#include "TestMacros.h"

TestMacros::TestMacros() : Test::Suite()
{
    TEST_ADD_WITH_STRING_LITERAL(TestMacros::testMethodWithCStringArg, (char*)"42");
    TEST_ADD_WITH_INTEGER(TestMacros::testMethodWithIntArg, 42);
    TEST_ADD_WITH_POINTER(TestMacros::testMethodWithPointerArg, nullptr);
    TEST_ADD_WITH_STRING(TestMacros::testMethodWithStringArg, "TestString");
    TEST_ADD_WITH_STRING(TestMacros::testMethodWithStringArg, std::string("TestString"));
    TEST_ADD(TestMacros::testFailureMessages);
}

void TestMacros::testMethodWithCStringArg(char* arg)
{
    TEST_ASSERT(strlen(arg) == 2);
}

void TestMacros::testMethodWithIntArg(const int arg)
{
    TEST_ASSERT_EQUALS(42, arg);
    TEST_PREDICATE([](int x){return x == 42;}, 42);
    TEST_BIPREDICATE([](int x, int y){return x == y;}, arg, arg);
}

void TestMacros::testMethodWithPointerArg(void* arg)
{
    TEST_ASSERT_EQUALS(nullptr, arg);
    TEST_ASSERT_EQUALS_MSG(nullptr, (void*)0xFF, "Should fail. 0xFF is not a nullptr");
}

void TestMacros::testMethodWithStringArg(std::string string)
{
    TEST_ASSERT(!string.empty());
    TEST_BIPREDICATE_MSG([](std::string s1, std::string s2){return s1.size() != s2.size();}, string, string, "Will fail");
}


void TestMacros::testFailureMessages()
{
    //test failure-messages for correctness/showcase their exactness
    int i = 42;
    TEST_ASSERT_MSG(i == 0, "Assertion should fail");
    TEST_ASSERT_EQUALS_MSG(i, 45, "Comparison should fail");
    TEST_ASSERT_DELTA_MSG(i, 45, 2, "Delta comparison should fail");
    
    TEST_THROWS_MSG(std::signbit(5), std::out_of_range, "Fails for not throwing exception");
    TEST_THROWS_MSG(throwsInt(), std::out_of_range, "Fails for non-exception-type");
    //XXX could use improvements
    TEST_THROWS_MSG(throwsException(), std::out_of_range, "Fails for throwing wrong exception");
    TEST_THROWS_ANYTHING_MSG(std::signbit(5), "Fails for throwing no exception");
    TEST_THROWS_ANYTHING_MSG(throwsInt(), "Fails for throwing a non-exception type");
    TEST_THROWS_NOTHING_MSG(throwsException(), "Fails for throwing exception");
    TEST_THROWS_NOTHING_MSG(throwsInt(), "Fails for throwing a non-exception type");
    
    TEST_PREDICATE_MSG(std::signbit, (double)i, "Fails for value not matching predicate");
    TEST_BIPREDICATE_MSG([](int x, int y){return x == y;}, i, 45, "Fails for not matching bipredicate");
    
    TEST_ABORT("Abort method");
    TEST_ASSERT_MSG(false,"Is not called");
}

void TestMacros::throwsException()
{
    throw std::invalid_argument("Some invalid argument");
}

void TestMacros::throwsInt()
{
    throw 5;
}

