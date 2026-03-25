#pragma once

#include <optional>
#include <vector>
#include <string>
#include <domain/user.hpp>

namespace car_rental::services {

enum class UserErrorCode {
    OK,
    VALIDATION_ERROR,
    CONFLICT,
    NOT_FOUND,
    INTERNAL_ERROR
};

struct UserResult {
    UserErrorCode code;
    std::string message;
    std::optional<domain::User> user;
};

struct UserListResult {
    UserErrorCode code;
    std::string message;
    std::vector<domain::User> users;
    int total;
};

class UserService {
public:
    static UserResult CreateUser(
        const std::string& login,
        const std::string& first_name,
        const std::string& last_name,
        const std::string& email,
        const std::string& phone,
        const std::string& password
    );
    
    static UserResult GetUserByLogin(const std::string& login);
    
    static UserListResult SearchUsers(
        const std::string& name_mask,
        int limit = 20,
        int offset = 0
    );
};

} // namespace car_rental::services