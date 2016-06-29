#include "crypto/CryptoSuite.h"

#if !defined CRYPTOPP_LIBRARY && !defined OPENSSL_CRYPTO_LIBRARY

#include <stdexcept>

using namespace ohmcomm::crypto;

const bool CryptoSuite::CRYPTO_SUITE_AVAILABLE = false;

CryptoSuite::CryptoSuite()
{
}

CryptoSuite::~CryptoSuite()
{
    
}

std::string CryptoSuite::hashMD5(const std::string& input)
{
    throw std::runtime_error("No crypto library configured!");
}

std::vector<unsigned char> CryptoSuite::createHMAC_SHA1(const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    throw std::runtime_error("No crypto library configured!");
}

bool CryptoSuite::verifyHMAC_SHA1(const void* hmacBuffer, const unsigned int hmacBufferSize, const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    throw std::runtime_error("No crypto library configured!");
}
#endif