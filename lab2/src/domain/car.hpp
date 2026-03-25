#pragma once

#include <string>
#include <chrono>

namespace car_rental::domain {

enum class CarClass {
    economy,
    compact,
    midsize,
    fullsize,
    luxury,
    suv,
    van
};

struct Car {
    std::string id;
    std::string vin;
    std::string brand;
    std::string model;
    int year;
    CarClass car_class;
    std::string license_plate;
    double daily_rate;
    bool available{true};
    std::chrono::system_clock::time_point created_at;
};

} // namespace car_rental::domain