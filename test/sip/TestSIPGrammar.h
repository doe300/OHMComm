/* 
 * File:   TestSIPGrammar.h
 * Author: daniel
 *
 * Created on February 13, 2016, 12:07 PM
 */

#ifndef TESTSIPGRAMMAR_H
#define	TESTSIPGRAMMAR_H

#include "cpptest.h"
#include "sip/SIPGrammar.h"

class TestSIPGrammar : public Test::Suite
{
public:
    TestSIPGrammar();

    void testSIPURI();
    
    void testNamedAddress();
    
    void testToViaAddress();
    
    void testIsValidCallID();
};

#endif	/* TESTSIPGRAMMAR_H */

