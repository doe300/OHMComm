/* 
 * File:   ParallelSuite.cpp
 * Author: daniel
 * 
 * Created on September 13, 2015, 5:14 PM
 */

#include "ParallelSuite.h"

using namespace Test;

ParallelSuite::ParallelSuite() : Suite()
{
}

ParallelSuite::ParallelSuite(const std::string& suiteName) : Suite(suiteName)
{
}

ParallelSuite::~ParallelSuite()
{
    delete output;
}


bool ParallelSuite::run(Output& output, bool continueAfterFail)
{
    this->continueAfterFail = continueAfterFail;
    this->output = new SynchronizedOutput(output);
    output.initializeSuite(suiteName, testMethods.size());
    if(setup())
    {
        //warn if test-methods are directly added
        if(!testMethods.empty())
        {
            Assertion notEmptyAssterion(suiteName.c_str(), 0, "Any test-methods directly added to a parallel-suite are not executed!");
            output.printFailure(notEmptyAssterion);
        }
        //run tear-down after all tests
        tear_down();
    }
    output.finishSuite(suiteName, testMethods.size(), 0, std::chrono::microseconds::zero());
    
    std::vector<std::future<bool>> results(subSuites.size());
    //run sub-suites
    for(unsigned int i = 0; i < subSuites.size(); i++)
    {
        results[i] = std::async(std::launch::async, &ParallelSuite::runSuite, this, i);
    }
    
    //join sub-suites
    for(std::future<bool>& result : results)
    {
        result.get();
    }
    
    return true;
}

bool ParallelSuite::runSuite(unsigned int suiteIndex)
{
    return subSuites[suiteIndex]->run(*output, continueAfterFail);
}
