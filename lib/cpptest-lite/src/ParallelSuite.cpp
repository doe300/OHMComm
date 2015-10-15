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
