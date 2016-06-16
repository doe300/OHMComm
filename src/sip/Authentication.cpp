/* 
 * File:   Authentication.cpp
 * Author: doe300
 * 
 * Created on June 13, 2016, 5:25 PM
 */

#include "sip/Authentication.h"
#include "Utility.h"
#include "Logger.h"

#include <iomanip>
#include <sstream>

#ifdef CRYPTO_MD5_HEADER //Only compile when Crypto++ is linked
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include CRYPTO_MD5_HEADER
#endif

using namespace ohmcomm::sip;

Authentication::Authentication() : isAuthenticated(false), expirationTime(std::chrono::system_clock::now())
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
    //TODO hash values
    //request-digest = " KD(H(A1), unq(nonce):H(A2))
    const std::string response = hashMD5(Utility::joinStrings({hashMD5(a1), nonce, hashMD5(a2)}, ":"));
    return Utility::joinStrings({"Digest ", "username=\"", userName, "\", realm=\"", realm, "\", nonce=\"", nonce, 
            "\", uri=\"", requestURI, "\", response=\"", response, "\", algorithm=", algorithm}, "");
}

#ifdef CRYPTO_MD5_HEADER //Only compile when Crypto++ is linked
std::string DigestAuthentication::hashMD5(const std::string& input) const
{
    unsigned char digest[ CryptoPP::Weak::MD5::DIGESTSIZE ];
    CryptoPP::Weak::MD5 hash;
    hash.CalculateDigest( digest, (const byte*)input.data(), input.size() );
    std::stringstream stream;
    for(unsigned int i = 0; i < sizeof(digest); ++i)
    {
        stream << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << (int)digest[i];
    }
    return stream.str();
}
#else
#warning "A hashing library (e.g. Crypto++) must be provided for MD5 to work!"

std::string DigestAuthentication::hashMD5(const std::string& input) const
{
    return input;
}
#endif