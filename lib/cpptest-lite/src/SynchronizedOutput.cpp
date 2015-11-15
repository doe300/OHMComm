/* 
 * File:   SynchronizedOutput.cpp
 * Author: daniel
 * 
 * Created on October 16, 2015, 11:44 AM
 */

#include "SynchronizedOutput.h"

using namespace Test;

SynchronizedOutput::SynchronizedOutput(Output& realOutput) : realOutput(realOutput)
{
    #ifdef _WIN32
    outputMutex = CreateMutex(nullptr, false, L"OutputMutex");
    #endif
}

SynchronizedOutput::~SynchronizedOutput()
{
}

void SynchronizedOutput::initializeSuite(const std::string& suiteName, const unsigned int numTests)
{
    lockMutex();
    realOutput.initializeSuite(suiteName, numTests);
    unlockMutex();
}

void SynchronizedOutput::finishSuite(const std::string& suiteName, const unsigned int numTests, const unsigned int numPositiveTests, const std::chrono::microseconds totalDuration)
{
    lockMutex();
    realOutput.finishSuite(suiteName, numTests, numPositiveTests, totalDuration);
    unlockMutex();
}

void SynchronizedOutput::initializeTestMethod(const std::string& suiteName, const std::string& methodName, const std::string& argString)
{
    lockMutex();
    realOutput.initializeTestMethod(suiteName, methodName, argString);
    unlockMutex();
}

void SynchronizedOutput::finishTestMethod(const std::string& suiteName, const std::string& methodName, const bool withSuccess)
{
    lockMutex();
    realOutput.finishTestMethod(suiteName, methodName, withSuccess);
    unlockMutex();
}

void SynchronizedOutput::printException(const std::string& suiteName, const std::string& methodName, const std::exception& ex)
{
    lockMutex();
    realOutput.printException(suiteName, methodName, ex);
    unlockMutex();
}

void SynchronizedOutput::printSuccess(const Assertion& assertion)
{
    lockMutex();
    realOutput.printSuccess(assertion);
    unlockMutex();
}


void SynchronizedOutput::printFailure(const Assertion& assertion)
{
    lockMutex();
    realOutput.printFailure(assertion);
    unlockMutex();
}



void SynchronizedOutput::lockMutex()
{
    #ifdef _WIN32
    WaitForSingleObject(outputMutex, INFINITE);
    #else
    outputMutex.lock();
    #endif
}

void SynchronizedOutput::unlockMutex()
{
    #ifdef _WIN32
    ReleaseMutex(outputMutex);
    #else
    outputMutex.unlock();
    #endif
}
