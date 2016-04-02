/* 
 * File:   SIPHandler.cpp
 * Author: daniel
 * 
 * Created on July 26, 2015, 12:44 PM
 */

#include <sstream>
#include <limits.h>

#include "sip/SIPPackageHandler.h"

using namespace ohmcomm::sip;

SIPPackageHandler::SIPPackageHandler()
{
}

const std::string SIPPackageHandler::createRequestPackage(const SIPRequestHeader& header, const std::string& requestBody)
{
    std::stringstream stream(std::ios_base::out);
    //Request-Line: COMMAND <space> REQUEST_URI <space> SIP_VERSION CRLF
    stream << header.requestCommand << " " << header.requestURI << " " << SIP_VERSION << CRLF;
    //write header-fields
    writeHeaderFields(stream, header.fields, requestBody.size());
    //write body
    stream << requestBody;
    return stream.str();
}

const std::string SIPPackageHandler::readRequestPackage(const void* sipPackage, unsigned int packageLength, SIPRequestHeader& readHeader) 
{
    std::vector<std::string> results = readPackage(sipPackage, packageLength, readHeader.fields);
    //check protocol version
    if(SIP_VERSION.compare(results[2])!=0)
    {
        throw std::invalid_argument(std::string("Invalid SIP protocol version: ") + results[2]);
    }
    readHeader.requestCommand = results[0];
    readHeader.requestURI = Utility::decodeURI(results[1]);
    //check for request URI syntax
    if(readHeader.requestURI.find("sip:") != 0 || readHeader.requestURI.find('@') == std::string::npos)
    {
        throw std::invalid_argument(std::string("Invalid request URI: ") + results[1]);
    }
    return results[3];
}

const std::string SIPPackageHandler::createResponsePackage(const SIPResponseHeader& header, const std::string& responseBody)
{
    std::stringstream stream(std::ios_base::out);
    //Response-Line: SIP_VERSION <space> STATUS_CODE <space> REASON_PHRASE CRLF
    stream << SIP_VERSION << " " << header.statusCode << " " << header.reasonPhrase << CRLF;
    //write header-fields
    writeHeaderFields(stream, header.fields, responseBody.size());
    //write body
    stream << responseBody;
    return stream.str();
}

const std::string SIPPackageHandler::readResponsePackage(const void* sipPackage, unsigned int packageLength, SIPResponseHeader& readHeader)
{
    std::vector<std::string> results = readPackage(sipPackage, packageLength, readHeader.fields);//check protocol version
    if(SIP_VERSION.compare(results[0])!=0)
    {
        throw std::invalid_argument(std::string("Invalid SIP protocol version: ") + results[0]);
    }
    readHeader.statusCode = atoi(results[1].c_str());
    readHeader.reasonPhrase = results[2];
    return results[3];
}

bool SIPPackageHandler::isResponsePackage(const void* buffer, const unsigned int bufferLength)
{
    //response: SIP_VERSION <space> STATUS_CODE <space> REASON_PHRASE CRLF
    std::string line((const char*)buffer, std::min(bufferLength, (unsigned int)255));
    return line.find(SIP_VERSION) == 0;
}

bool SIPPackageHandler::isRequestPackage(const void* buffer, const unsigned int bufferLength)
{
    //request: COMMAND <space> REQUEST_URI <space> SIP_VERSION CRLF
    std::string tmp((const char*)buffer, std::min(bufferLength, (unsigned int)255));
    std::string line = tmp.substr(0, tmp.find(CRLF));
    return line.find(SIP_VERSION) + SIP_VERSION.size() == line.size();
}

bool SIPPackageHandler::hasMultipartBody(const SIPHeader& header)
{
    return !header[SIP_HEADER_CONTENT_TYPE].empty() && header[SIP_HEADER_CONTENT_TYPE].find("multipart/") == 0;
}

std::map<std::string, std::string> SIPPackageHandler::readMultipartBody(const SIPHeader& header, const std::string& body)
{
    if(!hasMultipartBody(header))
    {
        //return single part with whole body
        return {{header[SIP_HEADER_CONTENT_TYPE], body}};
    }
    //get boundary string
    const std::string contentType = header[SIP_HEADER_CONTENT_TYPE];
    const std::string::size_type start = contentType.find("boundary=") + strlen("boundary=");
    //boundary may be surrounded by '"'
    const std::string boundary = std::string("--").append(contentType.substr((contentType[start] == '"' ? start+1 : start), (contentType[start] == '"' ? (contentType.find('"', start) - start) : std::string::npos)));
    
    std::map<std::string,std::string> parts;
    std::string::size_type index = 0;
    while(true)
    {
        index = body.find(boundary, index);
        if(index == std::string::npos)
        {
            return parts;
        }
        index += boundary.size();
        if(body.substr(index, 2).compare("--") == 0)
        {
            //we have found the last boundary and the end of the multi-part body
            return parts;
        }
        //content-type of part is next after the boundary, defaults to "text/plain"
        while(::isspace(body[index])) ++index;
        std::string partType("text/plain");
        if(Utility::equalsIgnoreCase(body.substr(index, SIP_HEADER_CONTENT_LENGTH.size()), SIP_HEADER_CONTENT_TYPE))
        {
            //skip Content-Type:
            index += SIP_HEADER_CONTENT_TYPE.size() + 1;
            //Content-Type could end with new-line or space
            partType = body.substr(index, body.find_first_of("\n \r", index) - index);
            index = body.find_first_of("\n \r", index) + 1;
        }
        //now the content is everything from index to the next boundary (or end)
        parts.insert({partType, body.substr(index, body.find(boundary, index) - index)});
    }
    return parts;
}

void SIPPackageHandler::checkSIPHeader(const SIPHeader* header)
{
    if(header == nullptr)
    {
        throw std::invalid_argument("Can't check nullptr!");
    }
    /*
     * Taken from RFC 3261 (Section 20)
     * 
     * This list is not exhaustive and only lists header-fields which are used in OHMComm
     * 
     * Header field          Request    Response
     * ___________________________________________________
     * Call-ID                mandatory  mandatory
     * Contact                mandatory  1xx, 2xx, 3xx,485
     * From                   mandatory  mandatory
     * To                     mandatory  mandatory
     * Via                    mandatory  mandatory
     * CSeq                   mandatory  mandatory
     */
    //1. check all required fields
    if(!header->hasKey(SIP_HEADER_CALL_ID))
        throw std::invalid_argument("Missing Call-ID header-field!");
    if(!header->hasKey(SIP_HEADER_FROM))
        throw std::invalid_argument("Missing From header-field!");
    if(!header->hasKey(SIP_HEADER_TO))
        throw std::invalid_argument("Missing To header-field!");
    if(!header->hasKey(SIP_HEADER_VIA))
        throw std::invalid_argument("Missing Via header-field!");
    if(!header->hasKey(SIP_HEADER_CSEQ))
        throw std::invalid_argument("Missing CSeq header-field!");
    if(dynamic_cast<const SIPRequestHeader*>(header) != nullptr)
    {
        if(!header->hasKey(SIP_HEADER_CONTACT))
            throw std::invalid_argument("Missing Contact header-field!");
        
    }
    //2. check syntax/values of required fields
    if(!SIPGrammar::isValidCallID(header->operator [](SIP_HEADER_CALL_ID)))
    {
        throw std::invalid_argument("Invalid Call-ID header-field!");
    }
    //From/To have identical syntax
    const std::string fromField = header->operator [](SIP_HEADER_FROM);
    const std::string toField = header->operator [](SIP_HEADER_TO);
    if(SIPGrammar::readNamedAddress(fromField, 0).uri.protocol.empty())
    {
        throw std::invalid_argument("Invalid From header-field!");
    }
    if(SIPGrammar::readNamedAddress(toField, 0).uri.protocol.empty())
    {
        throw std::invalid_argument("Invalid To header-field!");
    }
    const std::string viaField = header->operator [](SIP_HEADER_VIA);
    if(std::get<1>(SIPGrammar::readViaAddress(viaField, 0)).host.empty())
    {
        throw std::invalid_argument("Invalid Via header-field!");
    }
    const std::string cseqField = header->operator [](SIP_HEADER_CSEQ);
    if(!SIPGrammar::isValidCSeq(cseqField))
    {
        throw std::invalid_argument("Invalid CSeq header-field!");
    }
}

void SIPPackageHandler::writeHeaderFields(std::stringstream& stream, std::vector<HeaderField> fields, unsigned int contentSize)
{
    bool contentSizeSet = false;
    for(HeaderField field: fields)
    {
        if(SIP_HEADER_CONTENT_LENGTH == field.key)
        {
            contentSizeSet = true;
            field.value = std::to_string(contentSize);
        }
        //Header-Line: KEY <:> <space> VALUE CRLF
        stream << field.key << ": " << field.value << CRLF;
    }
    //set content-length header-field
    if(!contentSizeSet)
    {
        //Header-Line: KEY <:> <space> VALUE CRLF
        stream << SIP_HEADER_CONTENT_LENGTH << ": " << std::to_string(contentSize) << CRLF;
    }
    //Header ends with an empty line
    stream << CRLF;
}

std::vector<std::string> SIPPackageHandler::readPackage(const void* sipPackage, unsigned int packageLength, std::vector<HeaderField>& header)
{
    std::vector<std::string> results(4);
    //create string from the input buffer
    std::string package((char*)sipPackage, packageLength);
    std::string::size_type index = 0;
    unsigned int prevIndex = 0;
    //read request-command / response SIP-version
    index = package.find(' ');
    results.at(0) = package.substr(0, index);
    //skip space
    prevIndex = index+1;
    //read request-URI / response status-code
    index = package.find(' ', prevIndex);
    results.at(1) = package.substr(prevIndex, index - prevIndex);
    //skip space
    prevIndex = index+1;
    //read request protocol-version / response reason-phrase
    index = package.find(CRLF, prevIndex);
    results.at(2) = package.substr(prevIndex, index - prevIndex);
    //skip \r\n
    index += 2;
    //index is at start of header-line
    unsigned int contentLength = UINT_MAX;
    //reader headers until we meet an empty line
    while(CRLF.compare(package.substr(index, 2)) != 0)
    {
        prevIndex = index;
        index = package.find(CRLF, prevIndex);
        if(index == std::string::npos || index > package.size())
        {
            //this will only trigger for invalid SIP packages
            break;
        }
        HeaderField field;
        //TODO handling of compact headers
        field.fromString(package.substr(prevIndex, index - prevIndex), ':');
        header.push_back(field);
        //move index to beginning of next line
        index += 2;
        if(SIP_HEADER_CONTENT_LENGTH == field.key)
        {
            contentLength = atoi(field.value.c_str());
        }
    }
    //skip the empty line
    index += 2;
    if(contentLength != UINT_MAX && (packageLength - index) != 0)
    {
        if(contentLength > (packageLength - index))
        {
            throw std::invalid_argument("Content-Length header-field longer than message-body!");
        }
        //the remainder is the message-body
        results.at(3) = package.substr(index, contentLength);
    }
    else
    {
        //empty body
        results.at(3) = "";
    }
    return results;
}
