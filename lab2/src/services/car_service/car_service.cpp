#include "car_service.hpp"

#include <infrastructure/in_memory_storage/in_memory_storage.hpp>

#include <algorithm>

namespace car_rental::services {

namespace {
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
        if (car.IsAvailable()) {
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
        if (car.GetCarClass() == car_class) {
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

std::string dtoCarClassToString(lab2::car::CarClass сarClass) {
    switch (сarClass) {
        case lab2::car::CarClass::kEconomy:  return "economy";
        case lab2::car::CarClass::kCompact:  return "compact";
        case lab2::car::CarClass::kMidsize:  return "midsize";
        case lab2::car::CarClass::kFullsize: return "fullsize";
        case lab2::car::CarClass::kLuxury:   return "luxury";
        case lab2::car::CarClass::kSuv:      return "suv";
        case lab2::car::CarClass::kVan:      return "van";
        default: return "unknown";
    }
}
} // anonymous namespace


CarResult CarService::CreateCar(const lab2::car::CreateCarRequest& dto) {
    auto car_result = domain::Car::Create(
        dto.vin,
        dto.brand,
        dto.model,
        dto.year,
        domain::Car::CarClassFromString(dtoCarClassToString(dto.car_class)),
        dto.license_plate,
        dto.daily_rate
    );

    if (std::holds_alternative<exceptions::domain::ValidationError>(car_result)) {
        const auto& error = std::get<exceptions::domain::ValidationError>(car_result);
        return {
            CarErrorCode::VALIDATION_ERROR,
            error.message,
            std::nullopt
        };
    }

    const auto& car = std::get<domain::Car>(car_result);

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