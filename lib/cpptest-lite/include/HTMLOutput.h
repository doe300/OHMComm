/* 
 * File:   HTMLOutput.h
 * Author: daniel
 *
 * Created on September 20, 2015, 12:53 PM
 */

#ifndef HTMLOUTPUT_H
#define	HTMLOUTPUT_H

#include "cpptest.h"
#include "CollectorOutput.h"

#include <ostream>

namespace Test
{

    /*!
     * Prints a HTML-statistics to the output-stream
     */
    class HTMLOutput : public CollectorOutput
    {
    public:
        HTMLOutput();
        virtual ~HTMLOutput()
        { }
        
        /*!
         * Generates a HTML page with a <table> listing the result of the tests
         * 
         * \param stream The stream to generate the HTML-table to
         * \param includePassed Whether to include passed test-methods (methods without any error)
         * \param title The title of the result-page
         */
        void generate(std::ostream& stream, bool includePassed = true, const std::string& title = "Test-Results");
        
    private:
        void generateHeader(std::ostream& stream, const std::string& title);
        void generateSuitesTable(std::ostream& stream, bool includePassed);
        void generateTestsTable(std::ostream& stream, const SuiteInfo& suite, bool includePassed);
    };

}

#endif	/* HTMLOUTPUT_H */

