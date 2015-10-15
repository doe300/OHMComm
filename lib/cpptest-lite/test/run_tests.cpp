// ---
//
// $Id: mytest.cpp,v 1.5 2008/07/11 16:49:43 hartwork Exp $
//
// CppTest - A C++ Unit Testing Framework
// Copyright (c) 2003 Niklas Lundell
//
// ---
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// ---
//
// Test program demonstrating all assert types and output handlers.
//
// ---

#include <iostream>

#include "../include/cpptest.h"
#include "TestMacros.h"
#include "TestOutputs.h"

using namespace std;

// Tests unconditional fail asserts
//
class FailTestSuite : public Test::Suite
{
public:
	FailTestSuite() : Test::Suite("FailTestSuite")
	{
		TEST_ADD(FailTestSuite::success);
		TEST_ADD(FailTestSuite::always_fail);
	
	}
	
private:
	void success() {}
	
	void always_fail()
	{
		// This will always fail
		//
		TEST_FAIL("unconditional fail");
	}
};

// Tests compare asserts
//
class CompareTestSuite : public Test::Suite
{
public:
	CompareTestSuite() : Test::Suite("CompareTestSuite")
	{
		TEST_ADD(CompareTestSuite::success);
		TEST_ADD(CompareTestSuite::compare);
		TEST_ADD(CompareTestSuite::delta_compare);
	}
	
private:
	void success() {}
	
	void compare()
	{
		// Will succeed since the expression evaluates to true
		//
		TEST_ASSERT(1 + 1 == 2);
		
		// Will fail since the expression evaluates to false
		//
		TEST_ASSERT_MSG(0 == 1, "This test should fail. 0 != 1");
	}
	
	void delta_compare()
	{
		// Will succeed since the expression evaluates to true
		//
		TEST_ASSERT_DELTA(0.5, 0.7, 0.3);
                TEST_ASSERT_DELTA(0.5, 0.7, -0.3);
		
		// Will fail since the expression evaluates to false
		//
		TEST_ASSERT_DELTA_MSG(0.5, 0.7, 0.1, "This test should fail. (0.7 - 0.5) > 0.1");
	}
};

// Tests throw asserts
//
class ThrowTestSuite : public Test::Suite
{
public:
	ThrowTestSuite() : Test::Suite("ThrowTestSuite")
	{
		TEST_ADD(ThrowTestSuite::success);
		TEST_ADD(ThrowTestSuite::test_throw);
	}
	
private:
	void success() {}
	
	void test_throw()
	{
		// Will fail since the none of the functions throws anything
		//
		TEST_THROWS_MSG(func(), int, "Should fail. func() does not throw, expected int exception");
		TEST_THROWS_MSG(func_no_throw(), int, "Should fail. func_no_throw() does not throw, expected int exception");
		TEST_THROWS_ANYTHING_MSG(func(), "Should fail. func() does not throw, expected any exception");
		TEST_THROWS_ANYTHING_MSG(func_no_throw(), "Should fail. func_no_throw() does not throw, expected any exception");
		
		// Will succeed since none of the functions throws anything
		//
		TEST_THROWS_NOTHING(func());
		TEST_THROWS_NOTHING(func_no_throw());
		
		// Will succeed since func_throw_int() throws an int
		//
		TEST_THROWS(func_throw_int(), int);
		TEST_THROWS_ANYTHING(func_throw_int());
		
		// Will fail since func_throw_int() throws an int (not a float)
		//
		TEST_THROWS_MSG(func_throw_int(), float, "Should fail. func_throw_int() throws an int, expected a float exception");
		TEST_THROWS_NOTHING_MSG(func_throw_int(), "Should fail. func_throw_int() throws an int, expected no exception at all");
	}
	
	void func() {}
	void func_no_throw() throw() {}
	void func_throw_int() throw(int) { throw 13; }
};

enum OutputType
{
	Compiler,
	Html,
	TextTerse,
	TextVerbose
};

// Main test program
//
int
main(int argc, char* argv[])
{
    FailTestSuite fts;
    CompareTestSuite cts;
    ThrowTestSuite tts;
    TestMacros tms;
    TestOutputs tos;

    // Run the tests
    //
    Test::Output* output = new Test::TextOutput(Test::TextOutput::Verbose);
    fts.run(*output, true);
    cts.run(*output, true);
    tts.run(*output, true);
    tms.run(*output, true);
    std::cout << std::endl << std::endl << std::endl;
    tos.run(*output, true);

    delete output;
    
    return 0;
}

