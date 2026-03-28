#include "user_service.hpp"
#include <fmt/format.h>

#include <infrastructure/in_memory_storage/in_memory_storage.hpp>
#include <lib/uuid_generator/uuid_generator.hpp>

#include <algorithm>

namespace car_rental::services {

UserResult UserService::CreateUser(
    const lab2::user::CreateUserRequest& dto
) {
    auto user_result = domain::User::Create(
        dto.login,
        dto.first_name,
        dto.last_name,
        dto.email,
        dto.password,
        dto.phone
    );

    if (std::holds_alternative<exceptions::domain::ValidationError>(user_result)) {
        const auto& error = std::get<exceptions::domain::ValidationError>(user_result);

        return {
            UserErrorCode::VALIDATION_ERROR,
            error.message,
            std::nullopt
        };
    }

    const auto& user = std::get<domain::User>(user_result);
    
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