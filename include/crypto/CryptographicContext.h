/* 
 * File:   CryptographicContext.h
 * Author: doe300
 *
 * Created on June 25, 2016, 12:33 PM
 */

#ifndef CRYPTOGRAPHICCONTEXT_H
#define CRYPTOGRAPHICCONTEXT_H

#include <string>
#include <vector>

namespace ohmcomm
{
    namespace crypto
    {
        
        enum class CipherMode : unsigned char
        {
            COUNTER_MODE,
            F8
        };
        
        enum class HashAlgorithm : unsigned char
        {
            HMAC_SHA1
        };
        
        //Length of the encryption key (in bytes)
        const unsigned short KEY_LENGTH{16};
        
        //Length of the SRTCP authentication tag (in bytes)
        const unsigned short SRTCP_AUTH_TAG_LENGTH{10};
        //Length of the SRTP authentication key (in bytes)
        const unsigned short SRTP_AUTH_KEY_LENGTH{20};
        const unsigned short SRTCP_AUTH_KEY_LENGTH{20};

        /*!
         * A collection of settings determining the algorithms and modes to use
         * \since 1.0
         */
        struct CryptoMode
        {
            std::string name;
            //length (in bytes) of the master key
            unsigned short masterKeyLength;
            //length (in bytes) of the master salt
            unsigned short saltLength;
            //the cipher-mode to apply
            CipherMode cipherMode;
            //the hashing-algorithm to apply
            HashAlgorithm hashAlgorithm;
            //the length (in bytes) of the SRTP authentication tag
            unsigned short authenticationLength;
        };
        
        /*!
         * encryption algorithm: AES in counter mode (CM)
         * 
         * This is the default mode for SRTP
         */
        const CryptoMode AES_CM_128_HMAC_SHA1_80{"AES_CM_128_HMAC_SHA1_80", 16, 14, CipherMode::COUNTER_MODE, HashAlgorithm::HMAC_SHA1, 10};
        const CryptoMode AES_CM_128_HMAC_SHA1_32{"AES_CM_128_HMAC_SHA1_32", 16, 14, CipherMode::COUNTER_MODE, HashAlgorithm::HMAC_SHA1, 4};
        const CryptoMode F8_128_HMAC_SHA1_80{"F8_128_HMAC_SHA1_80", 16, 14, CipherMode::F8, HashAlgorithm::HMAC_SHA1, 10};

        /*!
         * Stores all cryptographic-related data
         * 
         * For available modes and values, see RFC 4568 and RFC 3711
         * 
         * \since 1.0
         */
        struct CryptographicContext
        {
            CryptoMode cryptoMode;
            std::vector<char> masterKey;
            std::vector<char> masterSalt;
        };
    }
}
#endif /* CRYPTOGRAPHICCONTEXT_H */

