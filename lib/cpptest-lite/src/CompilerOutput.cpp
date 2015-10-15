/* 
 * File:   CompilerOutput.cpp
 * Author: daniel
 * 
 * Created on September 14, 2015, 10:23 PM
 */

#include "CompilerOutput.h"

using namespace Test;

const std::string CompilerOutput::FORMAT_GCC("%file:%line: %text");
const std::string CompilerOutput::FORMAT_MSVC("%file(%line) : %text");
const std::string CompilerOutput::FORMAT_GENERIC = CompilerOutput::FORMAT_GCC;

const std::string CompilerOutput::ARG_FILE("%file");
const std::string CompilerOutput::ARG_LINE("%line");
const std::string CompilerOutput::ARG_TEXT("%text");

CompilerOutput::CompilerOutput(const std::string& format) : CompilerOutput(format, std::cout)
{
}

CompilerOutput::CompilerOutput(const std::string& format, std::ostream& stream) : Output(), format(format), stream(stream)
{

}

CompilerOutput::~CompilerOutput()
{
    stream.flush();
}

void CompilerOutput::printFailure(const Assertion& assertion)
{
    std::string formatString(format, 0);
    //replace %file
    unsigned int index = formatString.find(ARG_FILE);
    if(index != std::string::npos)
        formatString.replace(index, ARG_FILE.size(), assertion.file);
    //replace %line
    index = formatString.find(ARG_LINE);
    if(index != std::string::npos)
        formatString.replace(index, ARG_LINE.size(), std::to_string(assertion.lineNumber));
    //replace %text
    index = formatString.find(ARG_TEXT);
    if(index != std::string::npos)
        formatString.replace(index, ARG_TEXT.size(), (!assertion.errorMessage.empty() ? assertion.errorMessage : assertion.userMessage));
    stream << formatString << std::endl;
}

void CompilerOutput::printException(const std::string& suiteName, const std::string& methodName, const std::exception& ex)
{
    Assertion assertion(suiteName.data(), 0, ex.what(), methodName.data());
    printFailure(assertion);
}
