/* 
 * File:   TestUserInput.cpp
 * Author: daniel
 * 
 * Created on July 24, 2015, 9:47 AM
 */

#include "TestUserInput.h"

TestUserInput::TestUserInput()
{
    TEST_ADD(TestUserInput::redirectStdin);
    TEST_ADD(TestUserInput::testInputBoolean);
    TEST_ADD(TestUserInput::testInputNumber);
    TEST_ADD(TestUserInput::testInputString);
    TEST_ADD(TestUserInput::testSelectStringOption);
    TEST_ADD(TestUserInput::testSelectStringOptionIndex);
    TEST_ADD(TestUserInput::testSelectIntOption);
    TEST_ADD(TestUserInput::testSelectIntOptionIndex);
    TEST_ADD(TestUserInput::resetStdin);
}

void TestUserInput::testInputBoolean()
{
    testStream << "Yes" << std::endl;
    TEST_ASSERT_MSG(UserInput::inputBoolean("Type 'Yes'"), "Boolean input failed for : Yes");
    testStream << "This text should be ignored" << std::endl << "No" << std::endl;
    TEST_ASSERT_MSG(false == UserInput::inputBoolean("Type 'No'"), "Boolean input failed for : No");
    testStream << 'y' << std::endl;
    TEST_ASSERT_MSG(UserInput::inputBoolean("Type 'y'"), "Boolean input failed for : y");
    testStream << 'n' << std::endl;
    TEST_ASSERT_MSG(false == UserInput::inputBoolean("Type 'n'"), "Boolean input failed for : n");
}

void TestUserInput::testInputNumber()
{
    testStream << 123 << std::endl;
    TEST_ASSERT_EQUALS_MSG(123, UserInput::inputNumber("Type '123'", false, false), "Number input failed for: 123");
    testStream << 0 << std::endl;
    TEST_ASSERT_EQUALS_MSG(0, UserInput::inputNumber("Type '0'", true, false), "Number input failed for: 0");
    testStream << -123 << std::endl;
    TEST_ASSERT_EQUALS_MSG(-123, UserInput::inputNumber("Type '-123'", true, true), "Number input failed for: -123");
}

void TestUserInput::testInputString()
{
    testStream << "HelloWorld!" << std::endl;
    TEST_ASSERT_EQUALS_MSG(std::string("HelloWorld!"), UserInput::inputString("Type 'HelloWorld!'"), "String input failed for: HelloWorld!");
    testStream << std::endl;
    TEST_ASSERT_EQUALS_MSG(std::string(""), UserInput::inputString("Type nothing"), "String input failed for empty string");
}

void TestUserInput::testSelectStringOption()
{
    std::vector<std::string> options = {std::string("A"), std::string("B")};
    testStream << 0 << std::endl;
    TEST_ASSERT_EQUALS_MSG(std::string("A"), UserInput::selectOption("Type '0'", options, "A"), "String selection failed for: A");
    testStream << 5 << std::endl;
    TEST_ASSERT_EQUALS_MSG(std::string("B"), UserInput::selectOption("Type '5'", options, "B"), "String selection failed for default value");
}

void TestUserInput::testSelectStringOptionIndex()
{
    std::vector<std::string> options = {std::string("A"), std::string("B")};
    testStream << 0 << std::endl;
    TEST_ASSERT_EQUALS_MSG(0, UserInput::selectOptionIndex("Type '0'", options, 1), "String index selection failed for: 0");
    testStream << 5 << std::endl;
    TEST_ASSERT_EQUALS_MSG(1, UserInput::selectOptionIndex("Type '5'", options, 1), "String index selection failed for default index");
}

void TestUserInput::testSelectIntOption()
{
    std::vector<int> options = {5,8,7,9};
    testStream << 8 << std::endl;
    TEST_ASSERT_EQUALS_MSG(8, UserInput::selectOption("Type '8'", options, 5), "Int selection failed for: 8");
    testStream << 10 << std::endl;
    TEST_ASSERT_EQUALS_MSG(9, UserInput::selectOption("Type '10'", options, 9), "Int selection failed for default value");
}

void TestUserInput::testSelectIntOptionIndex()
{
    std::vector<int> options = {5,8,7,9};
    testStream << 1 << std::endl;
    TEST_ASSERT_EQUALS_MSG(1, UserInput::selectOptionIndex("Type '1'", options, 0), "Int index selection failed for: 1");
    testStream << 11 << std::endl;
    TEST_ASSERT_EQUALS_MSG(3, UserInput::selectOptionIndex("Type '11'", options, 3), "Int index selection failed for default index");
}

void TestUserInput::redirectStdin()
{
    //save original input buffer
    origStdinBuf = std::cin.rdbuf();
    testStream.clear();
    std::cin.rdbuf(testStream.rdbuf());
}

void TestUserInput::resetStdin()
{
    std::cin.rdbuf(origStdinBuf);
}
