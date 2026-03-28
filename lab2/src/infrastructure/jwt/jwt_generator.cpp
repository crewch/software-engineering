#include "jwt_generator.hpp"

#include <jwt-cpp/jwt.h>
#include <openssl/rand.h>
#include <userver/logging/log.hpp>

namespace lab2::infrastructure {

JwtTokenGenerator::JwtTokenGenerator(std::string private_key,
                                     std::string issuer,
                                     std::string audience)
    : private_key_(std::move(private_key)),
      issuer_(std::move(issuer)),
      audience_(std::move(audience)) {}

std::string JwtTokenGenerator::Generate(
    const std::string& user_id,
    std::chrono::seconds ttl) const {

    const auto now = std::chrono::system_clock::now();

    unsigned char nonce[16];
    RAND_bytes(nonce, sizeof(nonce));

    const std::string jti =
        ::jwt::base::encode<::jwt::alphabet::base64url>(
            std::string(reinterpret_cast<const char*>(nonce), sizeof(nonce)));
    
    return ::jwt::create()
        .set_type("JWT")
        .set_issuer(issuer_)
        .set_audience(audience_)
        .set_subject(user_id)
        .set_id(jti)
        .set_issued_at(now)
        .set_not_before(now)
        .set_expires_at(now + ttl)
        .sign(::jwt::algorithm::hs256(private_key_));
}

}
