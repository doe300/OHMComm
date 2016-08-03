
#include "crypto/CryptoSuite.h"

#if defined(ENABLE_CRYPTOGRAPHICS) && defined(CRYPTOPP_LIBRARY) //Only compile of Crypto++ is available

#include <sstream>
#include <iomanip>
#include <algorithm>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#if CRYPTOPP_LIBRARY == 2 //use headers from custom library
#include "../lib/cryptopp/md5.h"
#include "../lib/cryptopp/hmac.h"
#include "../lib/cryptopp/sha.h"
#include "../lib/cryptopp/aes.h"
#include "../lib/cryptopp/ccm.h"
#include "../lib/cryptopp/modes.h"
#else       //use system headers
#include <cryptopp/md5.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/aes.h>
#include <cryptopp/ccm.h>
#include <cryptopp/modes.h>
#endif

using namespace ohmcomm::crypto;

const bool CryptoSuite::CRYPTO_SUITE_AVAILABLE = true;
const std::string CryptoSuite::CRYPTO_SUITE_NAME = "Cryptopp";

std::string CryptoSuite::hashMD5(const std::string& input)
{
    byte digest[ CryptoPP::Weak::MD5::DIGESTSIZE ];
    CryptoPP::Weak::MD5 hash;
    hash.CalculateDigest( digest, (const byte*)input.data(), input.size() );
    std::stringstream stream;
    for(unsigned int i = 0; i < sizeof(digest); ++i)
    {
        stream << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << (int)digest[i];
    }
    return stream.str();
}

std::vector<byte> CryptoSuite::createHMAC_SHA1(const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    byte digest[CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE];
    CryptoPP::HMAC<CryptoPP::SHA1> hmac((const byte*)cryptoContext->masterKey.data(), cryptoContext->masterKey.size());
    hmac.CalculateDigest(digest, (const byte*)buffer, bufferSize);
    std::vector<byte> result(CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE);
    std::copy(digest, digest + CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE, result.data());
    return result;
}

bool CryptoSuite::verifyHMAC_SHA1(const void* hmacBuffer, const unsigned int hmacBufferSize, const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    CryptoPP::HMAC<CryptoPP::SHA1> hmac((const byte*)cryptoContext->masterKey.data(), cryptoContext->masterKey.size());
    return hmac.VerifyDigest((const byte*)hmacBuffer, (const byte*)buffer, bufferSize);
}

std::vector<byte> CryptoSuite::encryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<byte>& key)
{
    std::vector<byte> result(bufferSize, '\0');
    if(mode == ohmcomm::crypto::CipherMode::COUNTER_MODE)
    {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption algorithm((const byte*)key.data(), key.size());
        //FIXME fails with "AES/CTR: this object requires an IV"
        algorithm.ProcessData(result.data(), (const byte*)inputBuffer, bufferSize);
    }
    else
    {
        throw std::invalid_argument("Unknown cipher-mode!");
    }
    return result;
}

std::vector<byte> CryptoSuite::decryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<byte>& key)
{
    std::vector<byte> result(bufferSize, '\0');
    if(mode == ohmcomm::crypto::CipherMode::COUNTER_MODE)
    {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption algorithm((const byte*)key.data(), key.size());
        algorithm.ProcessData(result.data(), (const byte*)inputBuffer, bufferSize);
    }
    else
    {
        throw std::invalid_argument("Unknown cipher-mode!");
    }
    return result;
}
#endif