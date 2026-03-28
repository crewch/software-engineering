#pragma once

#include <optional>
#include <vector>
#include <string>
#include <domain/user.hpp>

#include <docs/definitions/user.hpp>
#include <docs/definitions/auth.hpp>
#include <infrastructure/jwt/jwt_generator.hpp>

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

enum class AuthErrorCode {
    OK,
    VALIDATION_ERROR,
    INVALID_CREDENTIALS,
    NOT_FOUND,
    INTERNAL_ERROR
};

struct AuthResult {
    AuthErrorCode code;
    std::string message;
    std::optional<std::string> token;
};

class UserService {
public:
    static UserResult CreateUser(
        const lab2::user::CreateUserRequest& dto
    );
    
    static UserResult GetUserByLogin(const std::string& login);
    
    static UserListResult SearchUsers(
        const std::string& name_mask,
        int limit = 20,
        int offset = 0
    );

    static AuthResult Login(
        const lab2::auth::LoginRequest& dto,
        const std::shared_ptr<lab2::infrastructure::JwtTokenGenerator> jwt_token_generator
    );
};

} // namespace car_rental::services