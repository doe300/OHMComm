/* 
 * File:   Authentication.cpp
 * Author: doe300
 * 
 * Created on June 13, 2016, 5:25 PM
 */

#include "sip/Authentication.h"
#include "Utility.h"
#include "Logger.h"
#include "CryptoSuite.h"

using namespace ohmcomm::sip;

Authentication::Authentication() : authenticated(false), expirationTime(std::chrono::system_clock::now())
{
}

Authentication::~Authentication()
{
}

std::unique_ptr<Authentication> Authentication::getAuthenticationMethod(const std::string& requestMethod, const std::string& requestURI, const std::string& headerField)
{
    if(Utility::trim(headerField).find("Basic") == 0)
    {
        ohmcomm::info("Authentication") << "Using basic authentication" << ohmcomm::endl;
        return std::unique_ptr<Authentication>(new BasicAuthentication());
    }
    if(Utility::trim(headerField).find("Digest") == 0)
    {
        std::string::size_type index = 0;
        //realm
        index = headerField.find("realm") + std::string("realm=").size();
        const std::string realm = Utility::replaceAll(headerField.substr(index, headerField.find_first_of(",\n\r", index) - index), "\"", "");
        //nonce
        index = headerField.find("nonce") + std::string("nonce=").size();
        const std::string nonce = Utility::replaceAll(headerField.substr(index, headerField.find_first_of(",\n\r", index) - index), "\"", "");
        //algorithm
        index = headerField.find("algorithm") + std::string("algorithm=").size();
        std::string algorithm;
        if(index < headerField.find("algorithm"))
        {
            algorithm = "MD5";
        }
        else
        {
            algorithm = Utility::replaceAll(headerField.substr(index, headerField.find_first_of(",\n\r", index) - index), "\"", "");
        }
        ohmcomm::info("Authentication") << "Using digest authentication" << ohmcomm::endl;
        return std::unique_ptr<Authentication>(new DigestAuthentication(requestMethod, requestURI, realm, nonce, algorithm));
    }
    
    ohmcomm::warn("Authentication") << "Unknown authentication method for header: " << headerField << ohmcomm::endl;
    return std::unique_ptr<Authentication>(nullptr);
}

bool Authentication::isAuthenticated() const
{
    return authenticated && expirationTime >= std::chrono::system_clock::now();
}

void Authentication::setAuthenticated(const std::chrono::system_clock::time_point expirationTime)
{
    authenticated = true;
    this->expirationTime = expirationTime;
}

bool Authentication::isExpired() const
{
    return expirationTime < std::chrono::system_clock::now();
}

std::chrono::system_clock::time_point Authentication::getExpirationTime() const
{
    return expirationTime;
}

BasicAuthentication::BasicAuthentication() : Authentication()
{
}

BasicAuthentication::~BasicAuthentication()
{
}

std::string BasicAuthentication::createAuthenticationHeader(const std::string& userName, const std::string& password)
{
    //"Basic" Base64(user:pass)
    return std::string("Basic ") + Utility::encodeBase64((userName + ":") + password);
}

DigestAuthentication::DigestAuthentication(const std::string& method, const std::string& requestURI, const std::string& realm, const std::string& nonce, const std::string& algorithm) : 
    Authentication(), method(method), requestURI(requestURI), realm(realm), nonce(nonce), algorithm(algorithm)
{
}

DigestAuthentication::~DigestAuthentication()
{
}

std::string DigestAuthentication::createAuthenticationHeader(const std::string& userName, const std::string& password)
{
    //H(data) = MD5(data)
    // and
    //KD(secret, data) = H(concat(secret, ":", data))
    //A1 = unq(username-value) ":" unq(realm-value) ":" passwd
    const std::string a1 = Utility::joinStrings({userName, realm,password}, ":");
    //A2 = Method ":" digest-uri-value
    const std::string a2 = Utility::joinStrings({method,requestURI}, ":");
    
    //TODO value for qop present
    //request-digest = " KD(H(A1), unq(nonce):H(A2))
    const std::string response = CryptoSuite::hashMD5(Utility::joinStrings({CryptoSuite::hashMD5(a1), nonce, CryptoSuite::hashMD5(a2)}, ":"));
    return Utility::joinStrings({"Digest ", "username=\"", userName, "\", realm=\"", realm, "\", nonce=\"", nonce, 
            "\", uri=\"", requestURI, "\", response=\"", response, "\", algorithm=", algorithm}, "");
}