#pragma once

#include <regex>
#include <string>
#include <chrono>
#include <optional>

#include <fmt/format.h>
#include <lib/uuid_generator/uuid_generator.hpp>
#include <variant>
#include <domain/exceptions.hpp>

namespace car_rental::domain {

class User {
public:
    static std::variant<exceptions::domain::ValidationError, User> Create(
        const std::string& login,
        const std::string& first_name,
        const std::string& last_name,
        const std::string& email,
        const std::string& password,
        const std::optional<std::string>& phone
    ) {
        if (!IsValidLogin(login)) {
            return exceptions::domain::ValidationError{
                "login",
                fmt::format("Login must be {}-{} characters (a-z, A-Z, 0-9, _, -)",
                        kMinLoginLength, kMaxLoginLength)
            };
        }
    
        if (first_name.empty() || first_name.length() > kMaxFirstNameLength) {
            return exceptions::domain::ValidationError{
                "first_name",
                fmt::format("First name must be 1-{} characters", kMaxFirstNameLength)
            };
        }
        
        if (last_name.empty() || last_name.length() > kMaxLastNameLength) {
            return exceptions::domain::ValidationError{
                "last_name",
                fmt::format("Last name must be 1-{} characters", kMaxLastNameLength)
            };
        }
        
        if (!IsValidEmail(email)) {
            return exceptions::domain::ValidationError{"email", "Invalid email format"};
        }
        
        if (phone.has_value() && !IsValidPhone(phone.value())) {
            return exceptions::domain::ValidationError{"phone", "Invalid phone format"};
        }
        
        User user;
        user.id_ = generateUUID();
        user.login_ = login;
        user.first_name_ = first_name;
        user.last_name_ = last_name;
        user.email_ = email;
        user.password_ = password;
        user.phone_ = phone;
        user.created_at_ = std::chrono::system_clock::now();
        
        return user;
    }

    const std::string& GetId() const { return id_; }
    const std::string& GetLogin() const { return login_; }
    const std::string& GetFirstName() const { return first_name_; }
    const std::string& GetLastName() const { return last_name_; }
    const std::string& GetEmail() const { return email_; }
    const std::optional<std::string>& GetPhone() const { return phone_; }
    const std::chrono::system_clock::time_point& GetCreatedAt() const { return created_at_; }

private:
    std::string id_;
    std::string login_;
    std::string first_name_;
    std::string last_name_;
    std::string email_;
    std::string password_;
    std::optional<std::string> phone_;
    std::chrono::system_clock::time_point created_at_;

    static constexpr int kMinLoginLength = 3;
    static constexpr int kMaxLoginLength = 50;
    static constexpr int kMaxFirstNameLength = 100;
    static constexpr int kMaxLastNameLength = 100;
    static constexpr int kMaxEmailLength = 255;
    static constexpr int kMinPasswordLength = 7;
    static constexpr int kMaxPasswordLength = 128;

    static bool IsValidLogin(const std::string& login) {
        if (login.length() < kMinLoginLength || login.length() > kMaxLoginLength) {
            return false;
        }
        
        static const std::regex login_pattern("^[a-zA-Z0-9_-]+$");
        return std::regex_match(login, login_pattern);
    }

    static bool IsValidEmail(const std::string& email) {
        if (email.empty() || email.length() > kMaxEmailLength) {
            return false;
        }
        
        return email.find('@') != std::string::npos &&
        email.find('.') != std::string::npos;
    }

    static bool IsValidPhone(const std::string& phone) {
        if (phone.empty()) {
            return true;
        }
        
        static const std::regex phone_pattern("^\\+?[0-9\\s\\-()]{10,20}$");
        return std::regex_match(phone, phone_pattern);
    }
};

} // namespace car_rental::domain