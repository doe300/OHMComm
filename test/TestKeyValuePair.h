/* 
 * File:   TestKeyValuePair.h
 * Author: doe300
 *
 * Created on June 19, 2016, 4:39 PM
 */

#ifndef TESTKEYVALUEPAIR_H
#define TESTKEYVALUEPAIR_H

#include "cpptest.h"
#include "KeyValuePairs.h"

class TestKeyValuePair : public Test::Suite
{
public:
    TestKeyValuePair();

    void testKeyValuePair();
    
    void testKeyValuePairs();
    
private:
    using PairType = ohmcomm::KeyValuePair<std::string>;
    using MapType = ohmcomm::KeyValuePairs<PairType>;
};

#endif /* TESTKEYVALUEPAIR_H */

