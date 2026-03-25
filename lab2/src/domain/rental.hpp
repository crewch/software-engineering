#pragma once

#include <string>
#include <chrono>
#include <optional>

namespace car_rental::domain {

enum class RentalStatus {
    active,
    completed,
    cancelled
};

struct Rental {
    std::string id;
    std::string user_id;
    std::string car_id;
    std::chrono::system_clock::time_point start_date;
    std::chrono::system_clock::time_point end_date;
    std::optional<std::chrono::system_clock::time_point> actual_return_date;
    double total_cost{0.0};
    RentalStatus status{RentalStatus::active};
    std::chrono::system_clock::time_point created_at;
};

} // namespace car_rental::domain