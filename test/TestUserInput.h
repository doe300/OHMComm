/*
 * File:   TestUserInput.h
 * Author: daniel
 *
 * Created on July 24, 2015, 9:47 AM
 */

#ifndef TESTUSERINPUT_H
#define	TESTUSERINPUT_H

#include <vector>

#include "cpptest.h"
#include "UserInput.h"

class TestUserInput : public Test::Suite
{
public:
    TestUserInput();

    void testInputBoolean();

    void testInputString();

    void testInputNumber();

    void testSelectStringOption();

    void testSelectStringOptionIndex();

    void testSelectIntOption();

    void testSelectIntOptionIndex();

    void redirectStdin();

    void resetStdin();
    
    bool before(const std::string& methodName);
private:
    void writeTestInput(std::string input);
    std::streambuf* origStdinBuf;
    std::stringstream testStream;
};

#endif	/* TESTUSERINPUT_H */

