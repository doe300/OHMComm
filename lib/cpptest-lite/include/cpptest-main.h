/* 
 * File:   cpptest-main.h
 * Author: daniel
 *
 * Created on September 12, 2015, 8:00 AM
 */

#ifndef CPPTEST_MAIN_H
#define	CPPTEST_MAIN_H

#include "cpptest.h"
#include <list>

namespace Test
{
    static bool continueAfterFailure = true;
    static Test::Output* cppTestOutput;
    static std::list<Test::Suite*> suites;
}

int main(int argc, char** argv)
{
    for(Test::Suite* suite : Test::suites)
    {
        suite->run(*Test::cppTestOutput, Test::continueAfterFailure);
    }
}

#define SET_OUTPUT(outputPtr) Test::cppTestOutput = outputPtr;
#define SET_CONTINUE_AFTER_FAILURE(continueAfterFailure) Test::continueAfterFailure = continueAfterFailure;
#define ADD_SUITE(suitePtr) Test::suites.push_back(suitePtr);

#endif	/* CPPTEST_MAIN_H */

