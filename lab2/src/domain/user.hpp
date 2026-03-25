#pragma once

#include <string>
#include <chrono>
#include <optional>

namespace car_rental::domain {

struct User {
    std::string id;
    std::string login;
    std::string first_name;
    std::string last_name;
    std::string email;
    std::optional<std::string> phone;
    std::chrono::system_clock::time_point created_at;
};

} // namespace car_rental::domain