/* 
 * File:   ParallelSuite.h
 * Author: daniel
 *
 * Created on September 13, 2015, 5:14 PM
 */

#ifndef PARALLELSUITE_H
#define	PARALLELSUITE_H

#include <thread>
#include <future>

#include "TestSuite.h"
#include "SynchronizedOutput.h"

namespace Test
{
    /*!
     * A suite which calls every sub-suite in an own thread and therefore in parallel.
     * ParallelSuite does not support adding test-methods directly to the suite. 
     * All test-methods must be added via a sub-suite
     */
    class ParallelSuite : public Suite
    {
    public:
        ParallelSuite();
        ParallelSuite(const std::string& suiteName);

        ~ParallelSuite();
        
        bool run(Output& output, bool continueAfterFail = true);

    private:
        bool runSuite(unsigned int suiteIndex);
    };
}

#endif	/* PARALLELSUITE_H */

