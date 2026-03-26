#include "car_service.hpp"

#include <infrastructure/in_memory_storage/in_memory_storage.hpp>

#include <regex>
#include <algorithm>

namespace car_rental::services {

namespace {

constexpr int VIN_LENGTH = 17;
constexpr int BRAND_MAX_LENGTH = 50;
constexpr int MODEL_MAX_LENGTH = 50;
constexpr int MIN_YEAR = 1900;
constexpr int MAX_YEAR = 2030;

bool IsValidVin(const std::string& vin) {
    if (vin.length() != VIN_LENGTH) {
        return false;
    }
    static const std::regex vin_pattern("^[A-HJ-NPR-Z0-9]{17}$");
    return std::regex_match(vin, vin_pattern);
}

std::vector<domain::Car> FilterByAvailability(
    const std::vector<domain::Car>& cars,
    bool available_only
) {
    if (!available_only) {
        return cars;
    }
    
    std::vector<domain::Car> filtered;
    filtered.reserve(cars.size());
    
    for (const auto& car : cars) {
        if (car.available) {
            filtered.push_back(car);
        }
    }
    
    return filtered;
}

std::vector<domain::Car> FilterByClass(
    const std::vector<domain::Car>& cars,
    domain::CarClass car_class
) {
    std::vector<domain::Car> filtered;
    filtered.reserve(cars.size());
    
    for (const auto& car : cars) {
        if (car.car_class == car_class) {
            filtered.push_back(car);
        }
    }
    
    return filtered;
}

std::vector<domain::Car> ApplyPagination(
    const std::vector<domain::Car>& cars,
    int limit,
    int offset,
    int& total_out
) {
    total_out = static_cast<int>(cars.size());
    
    if (offset >= total_out) {
        return {};
    }
    
    const int end = std::min(offset + limit, total_out);
    return std::vector<domain::Car>(cars.begin() + offset, cars.begin() + end);
}

} // anonymous namespace

CarResult CarService::CreateCar(const domain::Car& car) {
    if (!IsValidVin(car.vin)) {
        return {
            CarErrorCode::VALIDATION_ERROR,
            "VIN must be 17 characters (A-H, J-N, P, R, S, T, V, Z, 0-9)",
            std::nullopt
        };
    }
    
    if (car.brand.empty() || car.brand.length() > BRAND_MAX_LENGTH) {
        return {
            CarErrorCode::VALIDATION_ERROR,
            "Brand must be 1-50 characters",
            std::nullopt
        };
    }
    
    if (car.model.empty() || car.model.length() > MODEL_MAX_LENGTH) {
        return {
            CarErrorCode::VALIDATION_ERROR,
            "Model must be 1-50 characters",
            std::nullopt
        };
    }
    
    if (car.year < MIN_YEAR || car.year > MAX_YEAR) {
        return {
            CarErrorCode::VALIDATION_ERROR,
            "Year must be between 1900 and 2030",
            std::nullopt
        };
    }
    
    if (car.daily_rate < 0) {
        return {
            CarErrorCode::VALIDATION_ERROR,
            "Daily rate must be >= 0",
            std::nullopt
        };
    }

    auto& storage = storage::InMemoryStorage::Instance();
    
    if (!storage.CreateCar(car)) {
        return {
            CarErrorCode::CONFLICT,
            "Car with this VIN already exists",
            std::nullopt
        };
    }

    return {CarErrorCode::OK, "", car};
}

CarListResult CarService::GetAvailableCars(
    bool available_only,
    int limit,
    int offset
) {
    auto& storage = storage::InMemoryStorage::Instance();
    auto cars = storage.GetAllCars();

    cars = FilterByAvailability(cars, available_only);
    
    int total = 0;
    auto paginated = ApplyPagination(cars, limit, offset, total);

    return {CarErrorCode::OK, "", paginated, total};
}

CarListResult CarService::GetCarsByClass(
    domain::CarClass car_class,
    int limit,
    int offset
) {
    auto& storage = storage::InMemoryStorage::Instance();
    auto cars = storage.GetAllCars();

    cars = FilterByClass(cars, car_class);
    
    int total = 0;
    auto paginated = ApplyPagination(cars, limit, offset, total);

    return {CarErrorCode::OK, "", paginated, total};
}

} // namespace car_rental::services