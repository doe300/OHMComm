/* 
 * File:   SimpleTest.cpp
 * Author: daniel
 * 
 * Created on September 10, 2015, 11:28 AM
 */

#include "TestSuite.h"
#include <iostream>
#include <exception>

using namespace Test;

unsigned int Test::Suite::totalTestMethods = 0;

Suite::Suite() : Suite("")
{ }

Suite::Suite(const std::string& suiteName) : suiteName(suiteName), testMethods({}), subSuites({})
{ }

void Suite::add(std::shared_ptr<Test::Suite> suite)
{
    subSuites.push_back(suite);
}

bool Suite::run(Output& output, bool continueAfterFail)
{
    this->continueAfterFail = continueAfterFail;
    this->output = &output;
    output.initializeSuite(suiteName, testMethods.size());
    //run tests
    totalDuration = std::chrono::microseconds::zero();
    positiveTestMethods = 0;
    //run setup before all tests
    if(setup())
    {
        for(const TestMethod& method : testMethods)
        {
            std::pair<bool, std::chrono::microseconds> result = runTestMethod(method);
            totalDuration += result.second;
            if(result.first) ++positiveTestMethods;
        }
        //run tear-down after all tests
        tear_down();
    }
    output.finishSuite(suiteName, testMethods.size(), positiveTestMethods, totalDuration);
    
    //run sub-suites
    for(std::shared_ptr<Test::Suite>& suite : subSuites)
    {
        suite->run(output, continueAfterFail);
    }
    
    return positiveTestMethods == testMethods.size();
}

std::pair<bool, std::chrono::microseconds> Suite::runTestMethod(const TestMethod& method)
{
    errno = 0;
    bool exceptionThrown = false;
    currentTestMethodName = method.name;
    currentTestSucceeded = true;
    output->initializeTestMethod(suiteName, method.name, method.argString);
    //run before() before every test
    if(before(currentTestMethodName))
    {
        std::chrono::microseconds startTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch());
        try {
            method((Suite*)this);
        }
        catch(const std::exception& e)
        {
            exceptionThrown = true;
            currentTestSucceeded = false;
            output->printException(suiteName, method.name, e);
        }
        catch(...)
        {
            exceptionThrown = true;
            currentTestSucceeded = false;
            output->printException(suiteName, method.name, std::runtime_error("non-exception type thrown"));
        }
        std::chrono::microseconds endTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch());
        //run after() after every test
        after(currentTestMethodName, currentTestSucceeded);
        if(!exceptionThrown)
        {
            //we don't need to print twice, that the method has failed
            output->finishTestMethod(suiteName, method.name, currentTestSucceeded);
        }
        return std::make_pair(currentTestSucceeded, endTime - startTime);
    }
    return std::make_pair(false, std::chrono::microseconds::zero());
}



