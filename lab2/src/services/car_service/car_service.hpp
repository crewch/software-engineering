#pragma once

#include <optional>
#include <vector>
#include <domain/car.hpp>

namespace car_rental::services {

enum class CarErrorCode {
    OK,
    VALIDATION_ERROR,
    CONFLICT,
    NOT_FOUND,
    INTERNAL_ERROR
};

struct CarResult {
    CarErrorCode code;
    std::string message;
    std::optional<domain::Car> car;
};

struct CarListResult {
    CarErrorCode code;
    std::string message;
    std::vector<domain::Car> cars;
    int total;
};

class CarService {
public:
    static CarResult CreateCar(const domain::Car& car);
    
    static CarListResult GetAvailableCars(
        const std::optional<domain::CarClass>& car_class,
        bool available_only = true,
        int limit = 20,
        int offset = 0
    );
    
    static CarListResult GetCarsByClass(
        domain::CarClass car_class,
        bool available_only = true,
        int limit = 20,
        int offset = 0
    );
};

} // namespace car_rental::services