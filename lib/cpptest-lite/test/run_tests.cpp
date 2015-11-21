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
#include "TestParallelSuite.h"

using namespace std;

// Main test program
//
int main(int argc, char* argv[])
{
    FailTestSuite fts;
    CompareTestSuite cts;
    ThrowTestSuite tts;
    TestMacros tms;
    TestOutputs tos;
    TestParallelSuite tps;

    // Run the tests
    //
    Test::Output* output = new Test::TextOutput(Test::TextOutput::Verbose);
    fts.run(*output, true);
    cts.run(*output, true);
    tts.run(*output, true);
    tms.run(*output, true);
    std::cout << std::endl << std::endl << std::endl;
    tos.run(*output, true);
    std::cout << std::endl << std::endl << std::endl;
    tps.run(*output, true);

    delete output;
    
    return 0;
}

