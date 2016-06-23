/* 
 * File:   TestOutputs.cpp
 * Author: daniel
 * 
 * Created on September 20, 2015, 12:41 PM
 */

#include "TestOutputs.h"

#include <fstream>

using namespace Test;

TestOutputs::TestOutputs() : Test::Suite("TestOutputs"), textOutput(new TextOutput(TextOutput::Verbose)),
        compilerOutput(new CompilerOutput(CompilerOutput::FORMAT_GENERIC)), htmlOutput(new HTMLOutput()), consoleOutput(new ConsoleOutput(TextOutput::Verbose))
{
    TEST_ADD_WITH_POINTER(TestOutputs::testOutput, (void*)textOutput);
    TEST_ADD_WITH_POINTER(TestOutputs::testOutput, (void*)compilerOutput);
    TEST_ADD_WITH_POINTER(TestOutputs::testOutput, (void*)htmlOutput);
    TEST_ADD_WITH_POINTER(TestOutputs::testOutput, (void*)consoleOutput);
}

TestOutputs::~TestOutputs()
{
    delete textOutput;
    delete compilerOutput;
    delete htmlOutput;
    delete consoleOutput;
}


void TestOutputs::testOutput(void* output)
{
    Output* realOutput = (Output*)output;
    TestWithOutput runTest;
    runTest.run(*realOutput, true);
    HTMLOutput* htmlOutput = dynamic_cast<HTMLOutput*>(realOutput);
    if(htmlOutput != nullptr)
    {
        std::fstream htmlFile("result.html", std::ios_base::out);
        htmlOutput->generate(htmlFile, true);
        htmlFile.close();
    }
}

TestWithOutput::TestWithOutput() : Suite("TestWithOutput")
{
    //test Output-format
    TEST_ADD(TestWithOutput::someTestMethod);
    //tests overloaded test-methods
    TEST_ADD_WITH_STRING_LITERAL(TestWithOutput::anotherTestMethod, (char*)"Test method 1");
    TEST_ADD_WITH_STRING_LITERAL(TestWithOutput::anotherTestMethod, (char*)"Test method 2");
}

void TestWithOutput::someTestMethod()
{
    TEST_ASSERT(false);
    TEST_ASSERT_EQUALS(1, 4);
    
    TEST_FAIL("Failed test");
    
    TEST_ASSERT_MSG(true, "Passes");
}

void TestWithOutput::anotherTestMethod(char* someText)
{
    TEST_ASSERT_MSG(std::string(someText).empty(), "Should fail, text is not empty");
    
    TEST_ASSERT(std::string(someText).substr(0, 4).compare("Test") == 0);
    TEST_ASSERT_MSG(someText[12] == '1', "Fails in one case");
}


