/* 
 * File:   ParallelSuite.h
 * Author: daniel
 *
 * Created on September 13, 2015, 5:14 PM
 */

#ifndef PARALLELSUITE_H
#define	PARALLELSUITE_H

#include "TestSuite.h"

/*!
 * A suite which calls every sub-suite in an own thread and therefore in parallel
 */
namespace Test
{
    //TODO override all methods and make sure, access to shared data (i.e. output) is synchronized

    class ParallelSuite : public Suite
    {
    public:
        ParallelSuite();
        ParallelSuite(const std::string& suiteName);

        virtual ~ParallelSuite()
        {
        }
        
    protected:
    private:
    };

};

#endif	/* PARALLELSUITE_H */

