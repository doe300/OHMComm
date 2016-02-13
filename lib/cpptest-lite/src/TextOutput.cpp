/* 
 * File:   TextOutput.cpp
 * Author: daniel
 * 
 * Created on September 11, 2015, 7:07 PM
 */

#include "TextOutput.h"

#include <iostream>
#include <string.h>
#include <errno.h>

using namespace Test;

TextOutput::TextOutput(const unsigned int mode) : TextOutput(mode, std::cout)
{ }

TextOutput::TextOutput(const unsigned int mode, std::ostream& stream) : Output(), mode(mode), stream(stream)
{ }

TextOutput::~TextOutput()
{
    stream.flush();
}

void TextOutput::initializeSuite(const std::string& suiteName, const unsigned int numTests)
{
    if(mode <= Verbose)
        stream << "Running suite '" << suiteName << "' with " << numTests << " tests..." << std::endl;
}

void TextOutput::finishSuite(const std::string& suiteName, const unsigned int numTests, const unsigned int numPositiveTests, const std::chrono::microseconds totalDuration)
{
    if(mode <= Verbose || numTests != numPositiveTests)
    stream << "Suite '" << suiteName << "' finished, " << numPositiveTests << '/' << numTests << " successful (" << prettifyPercentage(numPositiveTests, numTests) << "%) in " << totalDuration.count() << " microseconds (" << totalDuration.count()/1000.0 << " ms)." << std::endl;
}

void TextOutput::initializeTestMethod(const std::string& suiteName, const std::string& methodName, const std::string& argString)
{
    if(mode <= Debug)
        stream << "Running method '" << methodName << "'" << (argString.empty() ? "" : "with argument: ") << argString << "..." << std::endl;;
}

void TextOutput::finishTestMethod(const std::string& suiteName, const std::string& methodName, const bool withSuccess)
{
    if(mode <= Debug || (mode <= Verbose && !withSuccess))
        stream  << "Test-method '" << methodName << "' finished with " << (withSuccess ? "success!" : "errors!") << std::endl;
}

void TextOutput::printSuccess(const Assertion& assertion)
{
    if(mode <= Debug)
        stream << "Test '" << assertion.method << "' line " << assertion.lineNumber << " successful!" << std::endl;
}

void TextOutput::printFailure(const Assertion& assertion)
{
    stream << "Test '" << assertion.method << '(' << (assertion.args.empty() ? "" : assertion.args) << ")' failed!" << std::endl;
    stream << "\tSuite: " << assertion.suite << std::endl;
    stream << "\tFile: " << Private::getFileName(assertion.file) << std::endl;
    stream << "\tLine: " << assertion.lineNumber << std::endl;
    if(!assertion.errorMessage.empty())
        stream << "\tFailure: " << assertion.errorMessage << std::endl;
    if(!assertion.userMessage.empty())
        stream << "\tMessage: " << assertion.userMessage << std::endl;
}

void TextOutput::printException(const std::string& suiteName, const std::string& methodName, const std::exception& ex)
{
    stream << "Test-method '" << methodName << "' failed with exception!" << std::endl;
    stream << "\tException: " << ex.what() << std::endl;
    stream << "\tErrno: " << errno << std::endl;
    stream << "\tError: " << strerror(errno) << std::endl;
}



