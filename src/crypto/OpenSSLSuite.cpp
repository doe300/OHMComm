
#include "crypto/CryptoSuite.h"

#ifdef OPENSSL_CRYPTO_LIBRARY //Only compile of OpenSSL crypto is available

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <string.h>

#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

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
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)input.data(), input.size(), digest);
    std::stringstream stream;
    for(unsigned int i = 0; i < sizeof(digest); ++i)
    {
        stream << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << (int)digest[i];
    }
    return stream.str();
}

std::vector<unsigned char> CryptoSuite::createHMAC_SHA1(const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    unsigned char digest[SHA_DIGEST_LENGTH];
    const EVP_MD* algorithm = EVP_sha1();
    HMAC(algorithm, cryptoContext->masterKey.data(), cryptoContext->masterKey.size(), (const unsigned char*)buffer, bufferSize, digest, nullptr);
    std::vector<unsigned char> result(SHA_DIGEST_LENGTH);
    std::copy(digest, digest + SHA_DIGEST_LENGTH, result.data());
    return result;
}

bool CryptoSuite::verifyHMAC_SHA1(const void* hmacBuffer, const unsigned int hmacBufferSize, const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    //TODO is there a better way to verify??
    const std::vector<unsigned char> compare = createHMAC_SHA1(buffer, bufferSize, cryptoContext);
    return memcmp(compare.data(), hmacBuffer, hmacBufferSize) == 0;
}

#endif