/* 
 * File:   TestOutputs.h
 * Author: daniel
 *
 * Created on September 20, 2015, 12:41 PM
 */

#ifndef TESTOUTPUTS_H
#define	TESTOUTPUTS_H

#include "cpptest.h"

using namespace Test;

class TestOutputs : public Test::Suite
{
public:
    TestOutputs();
    ~TestOutputs();
    
    void testOutput(void* output);
private:
    Output* textOutput;
    Output* compilerOutput;
    Output* htmlOutput;
    Output* consoleOutput;
};

class TestWithOutput : public Test::Suite
{
public:
    TestWithOutput();
    
    void someTestMethod();
    
    void anotherTestMethod(char* someText);
};

#endif	/* TESTOUTPUTS_H */

