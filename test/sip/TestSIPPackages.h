/* 
 * File:   TestSIPPackages.h
 * Author: doe300
 *
 * Created on April 18, 2016, 5:19 PM
 */

#ifndef TESTSIPPACKAGES_H
#define TESTSIPPACKAGES_H

#include "cpptest.h"

#include "sip/SIPPackageHandler.h"

class TestSIPPackages : public Test::Suite
{
public:
    TestSIPPackages();
    
    void testRequestPackage();
    void testResponsePackage();
    void testMultipartBody();
};

#endif /* TESTSIPPACKAGES_H */

