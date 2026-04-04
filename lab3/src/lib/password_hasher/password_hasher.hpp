#pragma once

#include <string>

namespace lab2::infrastructure {

class PasswordHasher {
public:
    std::string Hash(const std::string& raw);

    bool Verify(const std::string& raw, const std::string& hash);
};

}  // namespace lab2::infrastructure
