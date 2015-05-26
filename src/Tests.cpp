#include "Tests.h"

myTest::myTest() {
	TEST_ADD(myTest::function1_to_test_some_code);
	TEST_ADD(myTest::function2_to_test_some_code);
}

void myTest::function1_to_test_some_code()
{
	// Will succeed since the expression evaluates to true
	TEST_ASSERT(1 + 1 == 2)

	// Will fail since the expression evaluates to false
	TEST_ASSERT_MSG(0 == 1, "Omg 0 ist nicht 1");

}

void myTest::function2_to_test_some_code()
{

}