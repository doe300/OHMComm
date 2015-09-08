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

inline void TestUserInput::writeTestInput(std::string input)
{
    std::cout.flush();
    std::cout << std::endl << "Inputting: " << input << std::endl;
    testStream << input << std::endl;
}

void TestUserInput::testInputBoolean()
{
    writeTestInput("Yes");
    TEST_ASSERT_MSG(UserInput::inputBoolean("Type 'Yes'"), "Boolean input failed for : Yes");
    writeTestInput("Some text");
    writeTestInput("No");
    TEST_ASSERT_MSG(false == UserInput::inputBoolean("Type 'No'"), "Boolean input failed for : No");
    writeTestInput("y");
    TEST_ASSERT_MSG(UserInput::inputBoolean("Type 'y'"), "Boolean input failed for : y");
    writeTestInput("n");
    TEST_ASSERT_MSG(false == UserInput::inputBoolean("Type 'n'"), "Boolean input failed for : n");
    writeTestInput("dummy");
    TEST_ASSERT_MSG(UserInput::inputBoolean("Type 'dummy'", true, UserInput::INPUT_USE_DEFAULT), "Boolean failed for default value");
}

void TestUserInput::testInputNumber()
{
    writeTestInput("123");
    TEST_ASSERT_EQUALS(123, UserInput::inputNumber("Type '123'"));
    writeTestInput("0");
    TEST_ASSERT_EQUALS(0, UserInput::inputNumber("Type '0'", 0, UserInput::INPUT_ALLOW_ZERO));
    writeTestInput("-123");
    TEST_ASSERT_EQUALS(-123, UserInput::inputNumber("Type '-123'", 0, UserInput::INPUT_ALLOW_EMPTY|UserInput::INPUT_ALLOW_NEGATIVE));
    writeTestInput("wrong");
    TEST_ASSERT_EQUALS(123, UserInput::inputNumber("Type 'wrong'", 123, UserInput::INPUT_USE_DEFAULT));
    writeTestInput("-123");
    TEST_ASSERT_EQUALS(123, UserInput::inputNumber("Type '-123'", 123, UserInput::INPUT_USE_DEFAULT));
}

void TestUserInput::testInputString()
{
    writeTestInput("HelloWorld!");
    TEST_ASSERT_EQUALS(std::string("HelloWorld!"), UserInput::inputString("Type 'HelloWorld!'"));
    writeTestInput("");
    TEST_ASSERT_EQUALS(std::string(""), UserInput::inputString("Type nothing", "", UserInput::INPUT_ALLOW_EMPTY));
    writeTestInput("");
    TEST_ASSERT_EQUALS(std::string("Hello"), UserInput::inputString("Type nothing", "Hello", UserInput::INPUT_USE_DEFAULT));
    writeTestInput("");
    TEST_ASSERT_EQUALS(std::string(""), UserInput::inputString("Type nothing", "Hello", UserInput::INPUT_USE_DEFAULT|UserInput::INPUT_ALLOW_EMPTY));
}

void TestUserInput::testSelectStringOption()
{
    std::vector<std::string> options = {std::string("A"), std::string("B")};
    writeTestInput("0");
    TEST_ASSERT_EQUALS_MSG(std::string("A"), UserInput::selectOption("Type '0'", options, "A"), "String selection failed for: A");
    writeTestInput("5");
    TEST_ASSERT_EQUALS_MSG(std::string("B"), UserInput::selectOption("Type '5'", options, "B"), "String selection failed for default value");
}

void TestUserInput::testSelectStringOptionIndex()
{
    std::vector<std::string> options = {std::string("A"), std::string("B")};
    writeTestInput("0");
    TEST_ASSERT_EQUALS_MSG(0, UserInput::selectOptionIndex("Type '0'", options, 1), "String index selection failed for: 0");
    writeTestInput("5");
    TEST_ASSERT_EQUALS_MSG(1, UserInput::selectOptionIndex("Type '5'", options, 1), "String index selection failed for default index");
}

void TestUserInput::testSelectIntOption()
{
    std::vector<int> options = {5,8,7,9};
    writeTestInput("8");
    //TODO something is wrong here, does not receive correct number
    int result = UserInput::selectOption("Type '8'", options, 5); // Returns first value of options
    TEST_ASSERT_EQUALS_MSG(8, result, "Int selection failed for: 8");
    writeTestInput("10");
    result = UserInput::selectOption("Type '10'", options, 9); // Returns the default value
    TEST_ASSERT_EQUALS_MSG(9, result, "Int selection failed for default value");
}

void TestUserInput::testSelectIntOptionIndex()
{
    std::vector<int> options = {5,8,7,9};
    writeTestInput("1");
    TEST_ASSERT_EQUALS_MSG(1, UserInput::selectOptionIndex("Type '1'", options, 0), "Int index selection failed for: 1");
    writeTestInput("11");
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
