
#include "CryptoSuite.h"

#ifdef CRYPTOPP_LIBRARY //Only compile of Crypto++ is available

#include <sstream>
#include <iomanip>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>

using namespace ohmcomm;

CryptoSuite::CryptoSuite()
{
    
}

CryptoSuite::~CryptoSuite()
{

}

std::string CryptoSuite::hashMD5(const std::string& input)
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
        
#endif