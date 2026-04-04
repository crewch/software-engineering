#pragma once

#include <string>
#include <chrono>

namespace lab2::infrastructure {

class JwtTokenGenerator {
public:
    JwtTokenGenerator(std::string private_key,
                      std::string issuer,
                      std::string audience);

    std::string Generate(
        const std::string& user_id,
        std::chrono::seconds ttl = std::chrono::hours(1)) const;

private:
    std::string private_key_;
    std::string issuer_;
    std::string audience_;
};

}
