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
#include <chrono>

namespace ohmcomm
{
    namespace sip
    {

        /*!
         * HTTP/SIP authentication
         * 
         * see: RFC 2617 https://tools.ietf.org/html/rfc2617
         * 
         * \since 0.9
         */
        class Authentication
        {
        public:
            Authentication();
            virtual ~Authentication();
            
            /*!
             * \param requestMethod The request-method required to authenticate
             * 
             * \param requestURI The URI of the request to authenticate
             * 
             * \param headerField The value of the WWW-Authenticate header-field
             * 
             * \return the applicable authentication method or an empty pointer
             */
            static std::unique_ptr<Authentication> getAuthenticationMethod(const std::string& requestMethod, const std::string& requestURI, const std::string& headerField);
            
            /*!
             * \param userName The name to authenticate with
             * 
             * \param password The password to authenticate with
             * 
             * \return The value of the Authenticate header-field
             */
            virtual std::string createAuthenticationHeader(const std::string& userName, const std::string& password) = 0;
            
            /*!
             * Returns whether the authentication was successful.
             * 
             * NOTE: An expired authentication will be treated as not authenticated
             */
            bool isAuthenticated() const;
            
            /*!
             * Sets the authentication to successful and the time (in seconds) this authentication expires in
             * 
             * \param expirationTime The time (in seconds) from now, this authentication will expire on server-side
             */
            void setAuthenticated(const std::chrono::system_clock::time_point expirationTime);
            
            /*!
             * \return whether this authentication has already expired
             */
            bool isExpired() const;
            
            /*!
             * \return The time-point on which this authentication will expire
             */
            std::chrono::system_clock::time_point getExpirationTime() const;
            
        private:
            bool authenticated;
            //time of expiration, only valid, if isAuthenticated is true
            std::chrono::system_clock::time_point expirationTime;
        };
        
        /*!
         * Basic HTTP/SIP authentication
         * 
         * Sends a Base64 encoded concatenation of user-name and password
         * 
         * NOTE: SIP actually does not allow this to be used (only digest authentication), but it was so easy to implement...
         */
        class BasicAuthentication : public Authentication
        {
        public:
            BasicAuthentication();

            virtual ~BasicAuthentication();
            
            std::string createAuthenticationHeader(const std::string& userName, const std::string& password) override;
        };
        
        /*!
         * Digest HTTP/SIP authentication
         * 
         * Uses a challenge send by the server and responds with a hashed version of the answer
         * 
         * NOTE: The default hash, MD5, is considered to be a WEAK hash and should therefore not be used anymore, but SIP 
         * still requires it to be supported, so we do
         */
        class DigestAuthentication : public Authentication
        {
        public:
            DigestAuthentication(const std::string& method, const std::string& requestURI, const std::string& realm, const std::string& nonce, const std::string& algorithm);

            virtual ~DigestAuthentication();

            std::string createAuthenticationHeader(const std::string& userName, const std::string& password) override;
        private:
            const std::string method;
            const std::string requestURI;
            const std::string realm;
            const std::string nonce;
            const std::string algorithm;
        };
    }
}

#endif /* AUTHENTICATION_H */

