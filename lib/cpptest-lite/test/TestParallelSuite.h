/* 
 * File:   TestParallelSuite.h
 * Author: daniel
 *
 * Created on October 16, 2015, 1:10 PM
 */

#ifndef TESTPARALLELSUITE_H
#define	TESTPARALLELSUITE_H

#include "TestMacros.h"
#include "TestSuites.h"

class TestParallelSuite : public Test::ParallelSuite
{
public:
    TestParallelSuite();

};

#endif	/* TESTPARALLELSUITE_H */

