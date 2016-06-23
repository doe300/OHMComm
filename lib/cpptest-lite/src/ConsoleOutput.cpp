/* 
 * File:   ConsoleOutput.cpp
 * Author: doe300
 * 
 * Created on April 25, 2016, 4:44 PM
 */

#include "ConsoleOutput.h"

#include <iostream>
#include <string.h>

using namespace Test;

const std::string ConsoleOutput::errorColor("\033[31m");
const std::string ConsoleOutput::successColor("\033[32m");
const std::string ConsoleOutput::resetColors("\033[39;49m");

ConsoleOutput::ConsoleOutput(const unsigned int mode) : TextOutput(mode, std::cout)
{
}

ConsoleOutput::~ConsoleOutput()
{
}

void ConsoleOutput::finishSuite(const std::string& suiteName, const unsigned int numTests, const unsigned int numPositiveTests, const std::chrono::microseconds totalDuration)
{
    if(numTests != numPositiveTests)
    {
        stream << errorColor;
    }
    TextOutput::finishSuite(suiteName, numTests, numPositiveTests, totalDuration);
    stream << resetColors;
}

void ConsoleOutput::finishTestMethod(const std::string& suiteName, const std::string& methodName, const bool withSuccess)
{
    if(!withSuccess)
    {
        stream << errorColor;
    }
    TextOutput::finishTestMethod(suiteName, methodName, withSuccess);
    stream << resetColors;
}

void ConsoleOutput::printSuccess(const Assertion& assertion)
{
    stream << successColor;
    TextOutput::printSuccess(assertion);
    stream << resetColors;
}

void ConsoleOutput::printFailure(const Assertion& assertion)
{
    stream << errorColor;
    TextOutput::printFailure(assertion);
    stream << resetColors;
}

void ConsoleOutput::printException(const std::string& suiteName, const std::string& methodName, const std::exception& ex)
{
    stream << errorColor;
    TextOutput::printException(suiteName, methodName, ex);
    stream << resetColors;
}