#include "user_service.hpp"
#include <fmt/format.h>

#include <infrastructure/in_memory_storage/in_memory_storage.hpp>
#include <lib/uuid_generator/uuid_generator.hpp>

#include <regex>
#include <algorithm>

namespace car_rental::services {

namespace {

constexpr int kMinLoginLength = 3;
constexpr int kMaxLoginLength = 50;
constexpr int kMaxFirstNameLength = 100;
constexpr int kMaxLastNameLength = 100;
constexpr int kMaxEmailLength = 255;
constexpr int kMinPasswordLength = 8;
constexpr int kMaxPasswordLength = 128;

bool IsValidLogin(const std::string& login) {
    if (login.length() < kMinLoginLength || login.length() > kMaxLoginLength) {
        return false;
    }
    static const std::regex login_pattern("^[a-zA-Z0-9_-]+$");
    return std::regex_match(login, login_pattern);
}

bool IsValidEmail(const std::string& email) {
    if (email.empty() || email.length() > kMaxEmailLength) {
        return false;
    }
    return email.find('@') != std::string::npos &&
           email.find('.') != std::string::npos;
}

bool IsValidPhone(const std::string& phone) {
    if (phone.empty()) {
        return true;
    }
    static const std::regex phone_pattern("^\\+?[0-9\\s\\-()]{10,20}$");
    return std::regex_match(phone, phone_pattern);
}

bool IsValidPassword(const std::string& password) {
    return password.length() >= kMinPasswordLength &&
           password.length() <= kMaxPasswordLength;
}

} // anonymous namespace

UserResult UserService::CreateUser(
    const std::string& login,
    const std::string& first_name,
    const std::string& last_name,
    const std::string& email,
    const std::string& phone,
    const std::string& password
) {
    if (!IsValidLogin(login)) {
        return {
            UserErrorCode::VALIDATION_ERROR,
            fmt::format("Login must be {}-{} characters (a-z, A-Z, 0-9, _, -)",
                       kMinLoginLength, kMaxLoginLength),
            std::nullopt
        };
    }
    
    if (first_name.empty() || first_name.length() > kMaxFirstNameLength) {
        return {
            UserErrorCode::VALIDATION_ERROR,
            fmt::format("First name must be 1-{} characters", kMaxFirstNameLength),
            std::nullopt
        };
    }
    
    if (last_name.empty() || last_name.length() > kMaxLastNameLength) {
        return {
            UserErrorCode::VALIDATION_ERROR,
            fmt::format("Last name must be 1-{} characters", kMaxLastNameLength),
            std::nullopt
        };
    }
    
    if (!IsValidEmail(email)) {
        return {
            UserErrorCode::VALIDATION_ERROR,
            "Invalid email format",
            std::nullopt
        };
    }
    
    if (!IsValidPhone(phone)) {
        return {
            UserErrorCode::VALIDATION_ERROR,
            "Invalid phone format",
            std::nullopt
        };
    }
    
    if (!IsValidPassword(password)) {
        return {
            UserErrorCode::VALIDATION_ERROR,
            fmt::format("Password must be {}-{} characters",
                       kMinPasswordLength, kMaxPasswordLength),
            std::nullopt
        };
    }
    
    domain::User user;
    user.id = generateUUID();
    user.login = login;
    user.first_name = first_name;
    user.last_name = last_name;
    user.email = email;
    user.phone = phone.empty() ? std::nullopt : std::make_optional(phone);
    user.created_at = std::chrono::system_clock::now();
    
    auto& storage = storage::InMemoryStorage::Instance();
    if (!storage.CreateUser(user)) {
        return {
            UserErrorCode::CONFLICT,
            "User with this login already exists",
            std::nullopt
        };
    }
    
    return {UserErrorCode::OK, "", user};
}

UserResult UserService::GetUserByLogin(const std::string& login) {
    auto& storage = storage::InMemoryStorage::Instance();
    auto user = storage.GetUserByLogin(login);
    
    if (!user.has_value()) {
        return {
            UserErrorCode::NOT_FOUND,
            "User not found",
            std::nullopt
        };
    }
    
    return {UserErrorCode::OK, "", user};
}

UserListResult UserService::SearchUsers(
    const std::string& name_mask,
    int limit,
    int offset
) {
    auto& storage = storage::InMemoryStorage::Instance();
    auto users = storage.SearchUsers(name_mask);
    
    const int total = static_cast<int>(users.size());
    
    if (offset >= total) {
        return {UserErrorCode::OK, "", {}, total};
    }
    
    const int end = std::min(offset + limit, total);
    std::vector<domain::User> paginated(users.begin() + offset, users.begin() + end);
    
    return {UserErrorCode::OK, "", paginated, total};
}

} // namespace car_rental::services