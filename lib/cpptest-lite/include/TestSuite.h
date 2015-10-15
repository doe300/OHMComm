/* 
 * File:   SimpleTest.h
 * Author: daniel
 *
 * Created on September 10, 2015, 11:28 AM
 */

#ifndef TESTSUITE_H
#define	TESTSUITE_H

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <memory> //shared_ptr
#include <type_traits> //is_constructible

#include "Output.h"

namespace Test
{

    /*!
     * Any test-class must extend this class
     */
    class Suite
    {
    public:
        Suite();
        Suite(const std::string& suiteName);

        /*!
         * Adds another Test::Suite to this suite.
         * All associated suites will be run after this suite has finished
         * 
         */
        void add(std::shared_ptr<Test::Suite> suite);

        /*!
         * Runs all the registered test-methods in this suite
         * 
         * \param output The output to print the results to
         * \param continueAfterFail whether to continue running after a test failed
         */
        bool run(Output& output, bool continueAfterFail = true);

        static unsigned int getTotalNumberOfTests()
        {
            return totalTestMethods;
        }
        
        virtual ~Suite()
        {
        }

    protected:

        //! Test-method without any parameter
        using SimpleTestMethod = void (Suite::*)();
        //! Test-method with a single parameter of arbitrary type
        template<typename T>
        using SingleArgTestMethod = void (Suite::*)(const T arg);

        inline void setSuiteName(const std::string& filePath)
        {
            if (this->suiteName.empty()) {
                this->suiteName = Private::getFileName(filePath);
                this->suiteName = this->suiteName.substr(0, this->suiteName.find_last_of('.'));
            }
        }

        inline void addTest(SimpleTestMethod method, const std::string& funcName)
        {
            testMethods.push_back(TestMethod(funcName, method));
            totalTestMethods++;
        }

        template<typename T, typename U>
        inline void addTest(SingleArgTestMethod<T> method, const std::string& funcName, const U& arg)
        {
            static_assert(std::is_constructible<T, U>::value, "Can't construct method-parameter out of given type!");
            testMethods.push_back(TestMethod(funcName, method, {arg}));
            totalTestMethods++;
        }
        
        inline void testSucceeded(Assertion&& assertion)
        {
            assertion.method = currentTestMethodName;
            assertion.suite = suiteName;
            output->printSuccess(assertion);
        }

        inline void testFailed(Assertion&& assertion)
        {
            currentTestSucceeded = false;
            assertion.method = currentTestMethodName;
            assertion.suite = suiteName;
            output->printFailure(assertion);
        }

        inline bool continueAfterFailure()
        {
            return continueAfterFail;
        }

        /*!
         * This method can be overridden to execute code before the test-methods in this class are run
         * 
         * \return A boolean value, if false, the whole suite will not be executed (inclusive \ref tearDown)
         */
        virtual bool setup()
        {
            //does nothing by default
            return true;
        }

        /*!
         * This method can be overridden to run code after all the test-methods have finished
         */
        virtual void tear_down()
        {
            //does nothing by default
        }

        /*!
         * Override this method to run code before every test-method
         * 
         * \return A boolean value. If false, the test-method will not be executed (inclusive \ref after)
         */
        virtual bool before(const std::string& methodName)
        {
            //does nothing by default
            return true;
        }

        /*!
         * Override this method to run code after every test-method
         */
        virtual void after(const std::string& methodName, const bool success)
        {
            //does nothing by default
        }

        static unsigned int totalTestMethods;

    private:

        struct TestMethod
        {
            const std::string name;
            const std::function<void(Suite*) > functor;
            const std::string argString;

            TestMethod() : name("")
            {
            }

            TestMethod(const std::string& name, SimpleTestMethod method) : name(name), functor(method), argString({})
            {
            }

            template<typename T>
            TestMethod(const std::string& name, SingleArgTestMethod<T> method, const T& arg) : 
                name(name), functor(std::bind(method, std::placeholders::_1, arg)), argString(std::to_string(arg))
            {
            }
            
            inline void operator()(Suite* suite) const
            {
                functor(suite);
            }
        };
        std::string suiteName;
        std::vector<TestMethod> testMethods;
        std::vector<std::shared_ptr<Test::Suite>> subSuites;
        bool continueAfterFail;

        unsigned int positiveTestMethods;
        std::string currentTestMethodName;
        bool currentTestSucceeded;
        std::chrono::microseconds totalDuration;
        Output* output;

        std::pair<bool, std::chrono::microseconds> runTestMethod(const TestMethod& method);
    };

    /*!
     * Registers a simple test-method
     */
#define TEST_ADD(func) setSuiteName(__FILE__); addTest(static_cast<SimpleTestMethod>(&func), #func)
    /*!
     * Registers a test-method taking a single argument of type std::string or a c-style string-literal
     */
#define TEST_ADD_WITH_STRING(func, string) setSuiteName(__FILE__); addTest(static_cast<SingleArgTestMethod<std::basic_string<char>>>(&func), #func, string)
    /*!
     * Registers a test-method taking a single argument of type c-string
     */
#define TEST_ADD_WITH_STRING_LITERAL(func, stringLiteral) setSuiteName(__FILE__); addTest(static_cast<SingleArgTestMethod<char*>>(&func), #func, stringLiteral)
    /*!
     * Registers a test-method accepting a single argument of type int (or any type which can be coerced from int)
     */
#define TEST_ADD_WITH_INTEGER(func, number) setSuiteName(__FILE__); addTest(static_cast<SingleArgTestMethod<int>>(&func), #func, number)
    /*!
     * Registers a test-method which takes an argument to a pointer of arbitrary data
     */
#define TEST_ADD_WITH_POINTER(func, pointer) setSuiteName(__FILE__); addTest(static_cast<SingleArgTestMethod<void*>>(&func), #func, pointer)
    
};

#endif	/* TESTSUITE_H */

