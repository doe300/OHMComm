/* 
 * File:   HTMLOutput.cpp
 * Author: daniel
 * 
 * Created on September 20, 2015, 12:53 PM
 */

#include "HTMLOutput.h"

using namespace Test;

HTMLOutput::HTMLOutput()
{
}

void HTMLOutput::generate(std::ostream& stream, bool includePassed, const std::string& title)
{
    //1. print header (including style-information)
    generateHeader(stream, title);
    stream << "<body>";
    //2. print table
    generateSuitesTable(stream, includePassed);
    stream << "<br>" << std::endl;
    auto suiteInfo = suites.begin();
    while(suiteInfo != suites.end())
    {
        if(includePassed || suiteInfo->numTests != suiteInfo->numPositiveTests)
        {
            generateTestsTable(stream, *suiteInfo, includePassed);
            stream << "<br>" << std::endl;
        }
        ++suiteInfo;
    }
    stream << "</body></html>" << std::endl;
    //flush all data to the stream
    stream.flush();
}

void HTMLOutput::generateHeader(std::ostream& stream, const std::string& title)
{
    stream << "<!DOCTYPE html>" << std::endl
            << "<html><head><title>" << title << "</title>" << std::endl
            << "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />" << std::endl
            << "<meta name=\"generator\" content=\"CppTest-lite - http://github.com/doe300/cpptest-lite\" />" << std::endl
            << "<style type=\"text/css\">" << std::endl
            << "td { vertical-align: super}" << std::endl
            << ".message { display: block}" << std::endl
            << "table { border: 1px solid grey; border-spacing: 0px}" << std::endl
            << "td {border-top: 1px solid grey; padding: 2px}" << std::endl
            << "td:not(:last-child), th:not(:last-child) {border-right: 1px solid grey}" << std::endl
            << ".allPassed { background-color: green}" << std::endl
            << ".mostPassed { background-color: yellow}" << std::endl
            << ".manyPassed { background-color: orange}" << std::endl
            << ".somePassed { background-color: orangered}" << std::endl
            << ".fewPassed {background-color: red}" << std::endl
            << "</style>" << std::endl
            << "</head>" << std::endl;
}

inline std::string getCssClass(const unsigned int totalTests, const unsigned int passedTests)
{
    if(totalTests == passedTests)
    {
        return "allPassed";
    }
    if( passedTests >= (totalTests*0.75))
    {
        return "mostPassed";
    }
    if(passedTests >= (totalTests*0.5))
    {
        return "manyPassed";
    }
    if(passedTests >= (totalTests* 0.25))
    {
        return "somePassed";
    }
    return "fewPassed";
}

void HTMLOutput::generateSuitesTable(std::ostream& stream, bool includePassed)
{
    //header
    stream << "<table id='top' class='suites'><tr><th>Suite</th><th># Tests</th><th>Passed Tests</th><th>Duration</th></tr>" << std::endl;
    //content
    auto info = suites.begin();
    while(info != suites.end())
    {
        if(includePassed || info->numPositiveTests != info->numTests)
        {
            stream << "<tr><td><a href='#suite_" << info->suiteName << "'>" << info->suiteName << "</a></td>"
                    << "<td>" << info->numTests << "</td>"
                    << "<td class='" << getCssClass(info->numTests, info->numPositiveTests) << "'>"
                        << info->numPositiveTests << " (" << prettifyPercentage(info->numPositiveTests, info->numTests) << "%)</td>"
                    << "<td>" << info->suiteDuration.count()/1000.0 << " ms (" << info->suiteDuration.count()/1000000.0 << " s)</td>"
                    << "</tr>" << std::endl;
        }
        ++info;
    }
    stream << "</table>" << std::endl;
}

void HTMLOutput::generateTestsTable(std::ostream& stream, const SuiteInfo& suite, bool includePassed)
{
    stream << "<table id='suite_" << suite.suiteName << "'>"
            << "<tr><th>Test-method</th><th># Assertions</th><th>Passed Assertions</th><th>Failures</th></tr>" << std::endl;
    //content
    auto testMethod = suite.methods.begin();
    while(testMethod != suite.methods.end())
    {
        unsigned int totalAssertions = testMethod->passedAssertions.size() + testMethod->failedAssertions.size();
        stream << "<tr><td>" << stripMethodName(testMethod->methodName)  << "(" << truncateString(testMethod->argString, 20) << ")</td>"
                << "<td>" << totalAssertions << "</td>"
                << "<td class='" << getCssClass(totalAssertions, testMethod->passedAssertions.size()) << "'>"
                    << testMethod->passedAssertions.size() << " (" 
                    << prettifyPercentage(testMethod->passedAssertions.size(), totalAssertions) << "%)</td>"
                << "<td>";
        auto assertion = testMethod->failedAssertions.begin();
        while(assertion != testMethod->failedAssertions.end())
        {
            stream << "<span class='message'><a href='file://" << assertion->file << "'>" << assertion->file << "</a>: " << assertion->lineNumber << ": "
                    << truncateString((!assertion->errorMessage.empty() ? assertion->errorMessage : assertion->userMessage), 62)
                    << "</span>";
            ++assertion;
        }
        stream << "</td>"
                << "</tr>" << std::endl;
        ++testMethod;
    }
    stream << "</table>" << std::endl;
    stream << "<a href='#top'>Back to top</a>" << std::endl;
}


