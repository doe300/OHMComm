
#include "crypto/CryptoSuite.h"

#ifdef CRYPTOPP_LIBRARY //Only compile of Crypto++ is available

#include <sstream>
#include <iomanip>
#include <algorithm>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/aes.h>
#include <cryptopp/ccm.h>
#include <cryptopp/modes.h>

using namespace ohmcomm::crypto;

const bool CryptoSuite::CRYPTO_SUITE_AVAILABLE = true;

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

std::vector<unsigned char> CryptoSuite::createHMAC_SHA1(const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    unsigned char digest[CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE];
    CryptoPP::HMAC<CryptoPP::SHA1> hmac((const byte*)cryptoContext->masterKey.data(), cryptoContext->masterKey.size());
    hmac.CalculateDigest(digest, (const byte*)buffer, bufferSize);
    std::vector<unsigned char> result(CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE);
    std::copy(digest, digest + CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE, result.data());
    return result;
}

bool CryptoSuite::verifyHMAC_SHA1(const void* hmacBuffer, const unsigned int hmacBufferSize, const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    CryptoPP::HMAC<CryptoPP::SHA1> hmac((const byte*)cryptoContext->masterKey.data(), cryptoContext->masterKey.size());
    return hmac.VerifyDigest((const byte*)hmacBuffer, (const byte*)buffer, bufferSize);
}

std::vector<unsigned char> CryptoSuite::encryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<unsigned char> key)
{
    std::vector<unsigned char> result(bufferSize, '\0');
    if(mode == ohmcomm::crypto::CipherMode::COUNTER_MODE)
    {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption algorithm((const byte*)key.data(), key.size());
        algorithm.ProcessData(result.data(), (const byte*)inputBuffer, bufferSize);
    }
    return result;
}

std::vector<unsigned char> CryptoSuite::decryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<unsigned char> key)
{
    std::vector<unsigned char> result(bufferSize, '\0');
    if(mode == ohmcomm::crypto::CipherMode::COUNTER_MODE)
    {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption algorithm((const byte*)key.data(), key.size());
        algorithm.ProcessData(result.data(), (const byte*)inputBuffer, bufferSize);
    }
    return result;
}
#endif