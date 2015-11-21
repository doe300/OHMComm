/* 
 * File:   TestParallelSuite.cpp
 * Author: daniel
 * 
 * Created on October 16, 2015, 1:10 PM
 */

#include "TestParallelSuite.h"

TestParallelSuite::TestParallelSuite() : Test::ParallelSuite("TestParallel")
{
    add(std::shared_ptr<Test::Suite>(new FailTestSuite()));
    add(std::shared_ptr<Test::Suite>(new CompareTestSuite()));
    add(std::shared_ptr<Test::Suite>(new ThrowTestSuite()));
    add(std::shared_ptr<Test::Suite>(new TestMacros()));
}