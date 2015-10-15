/* 
 * File:   asserts.h
 * Author: daniel
 *
 * Created on September 10, 2015, 11:22 AM
 */

#ifndef ASSERTS_H
#define	ASSERTS_H

#include <sstream>

namespace std
{
    
    //support for enum-class, which can't be implicitly converted to int
    template<typename T>
    inline
    typename std::enable_if<std::is_enum<T>::value>::type
    printObj(const T val, ostringstream& stream)
    {
        stream << (int) val;
    }
    
    template<typename T>
    inline
    typename std::enable_if<!std::is_enum<T>::value>::type
    printObj(const T val, ostringstream& stream)
    {
        stream << val;
    }
    
    template<typename T>
    inline string to_string(const T val)
    {
        ostringstream tmpstream;
        printObj<T>(val, tmpstream);
        return tmpstream.str();
    }
    
    inline string to_string(const string val)
    {
        return val;
    }
    
    inline string to_string(const nullptr_t ptr)
    {
        return "(nullptr)";
    }
};

namespace Test
{
    template<typename T>
    struct Predicate
    {
        bool operator()(const T& obj) const;
    };
    
    template<typename T>
    struct BiPredicate
    {
        bool operator()(const T& obj1, const T& obj2) const;
    };
    
    template<typename T>
#ifndef _WIN32
    constexpr bool inMaxDistance(const T& source, const T& compare, const T& maxDistance)
    {
        static_assert(std::is_arithmetic<T>(), "Can only compare arithmetic types");
        return ((compare - source) * ( compare > source ? 1 : -1)) <= (maxDistance < 0 ? -maxDistance : maxDistance);
    }
#else 
    inline bool inMaxDistance(const T& source, const T& compare, const T& maxDistance)
    {
        return ((compare - source) * ( compare > source ? 1 : -1)) <= (maxDistance < 0 ? -maxDistance : maxDistance);
    }
#endif
    
    
    template<typename T>
#ifndef _WIN32
    constexpr bool inRange(const T& lowerLimit, const T& upperLimit, const T& value)
    {
        static_assert(std::is_arithmetic<T>(), "Can only compare arithmetic types");
        return lowerLimit < upperLimit ? (lowerLimit <= value && value <= upperLimit) : (upperLimit <= value && value <= lowerLimit);
    }
#else
    inline bool inRange(const T& lowerLimit, const T& upperLimit, const T& value)
    {
        return lowerLimit < upperLimit ? (lowerLimit <= value && value <= upperLimit) : (upperLimit <= value && value <= lowerLimit);
    }
#endif
    
    
    template<typename T>
    inline bool inRangeObject(const T& lowerLimit, const T& upperLimit, const T& value)
    {
        return (lowerLimit < upperLimit) ? (lowerLimit <= value && value <= upperLimit) : (upperLimit <= value && value <= lowerLimit);
    } 
    
    
////
//  All asserts from cpptest-1.1.2
////
    
    
#define TEST_FAIL(msg) \
    { \
        testFailed(Test::Assertion(__FILE__, __LINE__, ((msg) != 0 ? #msg : ""))); \
        if(!continueAfterFailure()) return; \
    }
    
#define TEST_ASSERT(condition) \
    { \
        if(false == (condition)) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Assertion '") + #condition + std::string("' failed"), "")); \
            if(!continueAfterFailure()) return; \
        } \
        else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }

#define TEST_ASSERT_MSG(condition, msg) \
    { \
        if(false == (condition)) { \
            testFailed(Test::Assertion( __FILE__, __LINE__, std::string("Assertion '") + #condition + std::string("' failed"), ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
        else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
    
#define TEST_ASSERT_EQUALS(expected, value) \
    { \
        if(expected != value) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Got ") + std::to_string(value) + std::string(", expected ") + std::to_string(expected), "")); \
            if(!continueAfterFailure()) return; \
        } \
        else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
#define TEST_ASSERT_EQUALS_MSG(expected, value, msg) \
    { \
        if(expected != value) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Got ") + std::to_string(value) + std::string(", expected ") + std::to_string(expected), ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
        else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
#define TEST_ASSERT_EQUALS_OBJ(expected, value) \
    { \
        static_assert(false, "This macro is deprecated, use TEST_ASSERT_EQUALS instead"); \
    }
#define TEST_ASSERT_EQUALS_OBJ_MSG(expected, value, msg) \
    { \
        static_assert(false, "This macro is deprecated, use TEST_ASSERT_EQUALS_MSG instead"); \
    }
#define TEST_ASSERT_DELTA(expected, value, delta) \
    { \
        if(!Test::inMaxDistance(expected, value, delta)) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Got ") + std::to_string(value) + std::string(", expected ") + std::to_string(expected) + std::string(" +/- ") + std::to_string(delta), "")); \
            if(!continueAfterFailure()) return; \
        } \
        else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
#define TEST_ASSERT_DELTA_MSG(expected, value, delta, msg) \
    { \
        if(!Test::inMaxDistance(expected, value, delta)) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Got ") + std::to_string(value) + std::string(", expected ") + std::to_string(expected) + std::string(" +/- ") + std::to_string(delta), ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
        else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
#define TEST_THROWS(expression, except) \
    { \
        try { \
            expression; \
            /*If we get here, no exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__,__LINE__, std::string("Expected exception of type '") + #except + std::string("' was not thrown!"), "")); \
            if(!continueAfterFailure()) return; \
        } \
        catch(except) { testSucceeded(Test::Assertion(__FILE__,__LINE__)); } \
        catch(std::exception &ex) { \
            /*If we get here, wrong exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Wrong Exception was thrown: ") + ex.what(), "")); \
            if(!continueAfterFailure()) return; \
        } \
        catch(...) { \
            /* Any other type than an exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("A non-exception-type was thrown, expected exception of type: ") + #except, "")); \
            if(!continueAfterFailure()) return; \
        } \
    }
#define TEST_THROWS_MSG(expression, except, msg) \
    { \
        try { \
            expression; \
            /*If we get here, no exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Expected exception of type '") + #except + std::string("' was not thrown!"), ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
        catch(except) { testSucceeded(Test::Assertion(__FILE__,__LINE__)); } \
        catch(std::exception &ex) { \
            /*If we get here, wrong exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Wrong Exception was thrown: ") + ex.what(), ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
        catch(...) { \
            /* Any other type than an exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("A non-exception-type was thrown, expected exception of type: ") + #except, ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
    }
#define TEST_THROWS_ANYTHING(expression) \
    { \
        try { \
            expression; \
            /*If we get here, no exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, "Expected exception, nothing was thrown!", "")); \
            if(!continueAfterFailure()) return; \
        } \
        catch(std::exception &ex) { \
            testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
        } \
        catch(...) { \
            /* Any other type than an exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, "A non-exception-type was thrown!", "")); \
            if(!continueAfterFailure()) return; \
        } \
    }
#define TEST_THROWS_ANYTHING_MSG(expression, msg) \
    { \
        try { \
            expression; \
            /*If we get here, no exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, "Expected exception, nothing was thrown!", ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
        catch(std::exception &ex) { \
            testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
        } \
        catch(...) { \
            /* Any other type than an exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, "A non-exception-type was thrown!", ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
    }
#define TEST_THROWS_NOTHING(expression) \
    { \
        try { \
            expression; \
            /*If we get here, no exception was thrown*/ \
            testSucceeded(Test::Assertion(__FILE__, __LINE__)); \
        } \
        catch(std::exception &ex) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Exception thrown: ") + ex.what(), "")); \
            if(!continueAfterFailure()) return; \
        } \
        catch(...) { \
            /* Any other type than an exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, "A non-exception-type was thrown!", "")); \
            if(!continueAfterFailure()) return; \
        } \
    }
#define TEST_THROWS_NOTHING_MSG(expression, msg) \
    { \
        try { \
            expression; \
            /*If we get here, no exception was thrown*/ \
            testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
        } \
        catch(std::exception &ex) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Exception thrown: ") + ex.what(), ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
        catch(...) { \
            /* Any other type than an exception was thrown*/ \
            testFailed(Test::Assertion(__FILE__, __LINE__, "A non-exception-type was thrown!", ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } \
    }

////
//  Custom asserts
////
#define TEST_PREDICATE(predicate, value) \
    { \
        if(false == predicate(value)) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Value '") + std::to_string(value) + std::string("' did not match the predicate: ") + #predicate, "")); \
            if(!continueAfterFailure()) return; \
        } else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }   
#define TEST_PREDICATE_MSG(predicate, value, msg) \
    { \
        if(false == predicate(value)) {\
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Value '") + std::to_string(value) + std::string("' did not match the predicate: ") + #predicate, ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
#define TEST_BIPREDICATE(bipredicate, value0, value1) \
    { \
        if(false == bipredicate(value0, value1)) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Values '") + std::to_string(value0) + "' and '" + std::to_string(value1) + std::string("' did not match the bi-predicate: ") + #bipredicate, "")); \
            if(!continueAfterFailure()) return; \
        } else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
#define TEST_BIPREDICATE_MSG(bipredicate, value0, value1, msg) \
    { \
        if(false == bipredicate(value0, value1)) { \
            testFailed(Test::Assertion(__FILE__, __LINE__, std::string("Values '") + std::to_string(value0) + "' and '" + std::to_string(value1) + std::string("' did not match the bi-predicate: ") + #bipredicate, ((msg) != 0 ? #msg : ""))); \
            if(!continueAfterFailure()) return; \
        } else testSucceeded(Test::Assertion(__FILE__,__LINE__)); \
    }
    
#define TEST_ABORT(msg) \
    { \
        testFailed(Test::Assertion(__FILE__, __LINE__, "Test-method aborted!", ((msg) != 0 ? #msg : ""))); \
        return; \
    }
}   // end of namespace Test
#endif	/* ASSERTS_H */

