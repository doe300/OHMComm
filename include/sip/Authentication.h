/* 
 * File:   Authentication.h
 * Author: doe300
 *
 * Created on June 13, 2016, 5:25 PM
 */

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <string>
#include <memory>

namespace ohmcomm
{
    namespace sip
    {

        /*!
         * HTTP/SIP authentication
         * 
         * see: RFC 2617 https://tools.ietf.org/html/rfc2617
         */
        class Authentication
        {
        public:
            Authentication();
            virtual ~Authentication();
            
            static std::unique_ptr<Authentication> getAuthenticationMethod(const std::string headerField);
            
            virtual std::string createAuthenticationHeader(const std::string& userName, const std::string& password) = 0;
            
            bool isAuthenticated;
        };
        
        class BasicAuthentication : public Authentication
        {
        public:
            BasicAuthentication();

            virtual ~BasicAuthentication();
            
            std::string createAuthenticationHeader(const std::string& userName, const std::string& password) override;
        };
        
        class DigestAuthentication : public Authentication
        {
        public:
            DigestAuthentication(const std::string& realm, const std::string& nonce, const std::string& algorithm);

            virtual ~DigestAuthentication();

            std::string createAuthenticationHeader(const std::string& userName, const std::string& password) override;
        private:
            const std::string realm;
            const std::string nonce;
            const std::string algorithm;
        };
    }
}

#endif /* AUTHENTICATION_H */

