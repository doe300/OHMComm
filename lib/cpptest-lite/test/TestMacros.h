/* 
 * File:   TestMacros.h
 * Author: daniel
 *
 * Created on September 15, 2015, 6:27 PM
 */

#ifndef TESTMACROS_H
#define	TESTMACROS_H

#include <exception>
#include <cmath>

#include "cpptest.h"

class TestMacros : public Test::Suite
{
public:
    TestMacros();
    
    void testMethodNoArgs();
    void testMethodWithCStringArg(char* arg);
    void testMethodWithIntArg(const int arg);
    void testMethodWithPointerArg(void* arg);
    void testMethodWithStringArg(std::string string);
    void testMethodWithVarargs1(const double d, const int i);
    void testMethodWithVarargs2(std::string s, int i, double d);
    
    void testFailureMessages();
private:
    void throwsException();
    void throwsInt();
    
};

#endif	/* TESTMACROS_H */

