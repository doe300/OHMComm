/* 
 * File:   TestUtility.h
 * Author: daniel
 *
 * Created on December 9, 2015, 4:28 PM
 */

#ifndef TESTUTILITY_H
#define	TESTUTILITY_H

#include "cpptest.h"

#include "Utility.h"

class TestUtility : public Test::Suite
{
public:
    TestUtility();

    void printDomainInfo();
    
    void testNetworkInfo();
    
    void testAddressFromHostName();
    
    void testTrim();
    
    void testEqualsIgnoreCase();
    
    void testReplaceAll();
    
    void testJoinStrings();
    
    void testDecodeURI();
    
    void testToHexString();
};

#endif	/* TESTUTILITY_H */

