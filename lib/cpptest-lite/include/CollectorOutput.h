/* 
 * File:   CollectorOutput.h
 * Author: daniel
 *
 * Created on September 20, 2015, 12:16 PM
 */

#ifndef COLLECTOROUTPUT_H
#define	COLLECTOROUTPUT_H

#include "Output.h"

#include <vector>

namespace Test
{

    /*!
     * A collector-output collects all information to be processed after all tests have run
     */
    class CollectorOutput : public Output
    {
    public:
        CollectorOutput();
        virtual ~CollectorOutput();
        
        virtual void initializeSuite(const std::string& suiteName, const unsigned int numTests);
        virtual void finishSuite(const std::string& suiteName, const unsigned int numTests, const unsigned int numPositiveTests, const std::chrono::microseconds totalDuration);
        
        virtual void initializeTestMethod(const std::string& suiteName, const std::string& methodName, const std::string& argString);
        virtual void finishTestMethod(const std::string& suiteName, const std::string& methodName, const bool withSuccess);

        virtual void printException(const std::string& suiteName, const std::string& methodName, const std::exception& ex);
        virtual void printSuccess(const Assertion& assertion);
        virtual void printFailure(const Assertion& assertion);
        
    protected:
        
        struct TestMethodInfo
        {
            const std::string methodName;
            const std::string argString;
            bool withSuccess;
            std::vector<Assertion> failedAssertions;
            std::vector<Assertion> passedAssertions;
            std::string exceptionMessage;
            
            TestMethodInfo(const std::string& name, const std::string& argString) : methodName(name), argString(argString),
                withSuccess(false), failedAssertions({}), passedAssertions({}), exceptionMessage("")
            { }
        };
        
        struct SuiteInfo
        {
            const std::string suiteName;
            const unsigned int numTests;
            unsigned int numPositiveTests;
            std::chrono::microseconds suiteDuration;
            std::vector<TestMethodInfo> methods;
            
            SuiteInfo(const std::string& name, const unsigned int numTests) : suiteName(name), numTests(numTests), 
                numPositiveTests(0), suiteDuration(std::chrono::microseconds::zero()), methods({})
            {
                
            };
        };
        
        std::vector<SuiteInfo> suites;
    private:
        //we need pointers to the current suite/test-method, because their name is not unique
        SuiteInfo* currentSuite = nullptr;
        TestMethodInfo* currentMethod = nullptr;
    };
    
};

#endif	/* COLLECTOROUTPUT_H */

