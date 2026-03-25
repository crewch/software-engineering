#include "rental_service.hpp"

#include <infrastructure/in_memory_storage/in_memory_storage.hpp>
#include <lib/uuid_generator/uuid_generator.hpp>

#include <chrono>
#include <cmath>

namespace car_rental::services {

namespace {

constexpr int kHoursPerDay = 24;

int CalculateDays(
    std::chrono::system_clock::time_point start_date,
    std::chrono::system_clock::time_point end_date
) {
    const auto duration = std::chrono::duration_cast<std::chrono::hours>(
        end_date - start_date
    ).count();
    
    return (duration + kHoursPerDay - 1) / kHoursPerDay;
}

double CalculateTotalCost(double daily_rate, int days) {
    return daily_rate * static_cast<double>(days);
}

} // anonymous namespace

RentalResult RentalService::CreateRental(
    const std::string& user_id,
    const std::string& car_id,
    std::chrono::system_clock::time_point start_date,
    std::chrono::system_clock::time_point end_date
) {
    if (start_date >= end_date) {
        return {
            RentalErrorCode::VALIDATION_ERROR,
            "End date must be after start date",
            std::nullopt
        };
    }
    
    auto& storage = storage::InMemoryStorage::Instance();
    
    if (!storage.UserExists(user_id)) {
        return {
            RentalErrorCode::NOT_FOUND,
            "User not found",
            std::nullopt
        };
    }
    
    auto car = storage.GetCarById(car_id);
    if (!car.has_value()) {
        return {
            RentalErrorCode::NOT_FOUND,
            "Car not found",
            std::nullopt
        };
    }
    
    if (!storage.IsCarAvailable(car_id, start_date, end_date)) {
        return {
            RentalErrorCode::CAR_NOT_AVAILABLE,
            "Car is not available for selected period",
            std::nullopt
        };
    }
    
    const int days = CalculateDays(start_date, end_date);
    const double total_cost = CalculateTotalCost(car->daily_rate, days);
    
    domain::Rental rental;
    rental.id = generateUUID();
    rental.user_id = user_id;
    rental.car_id = car_id;
    rental.start_date = start_date;
    rental.end_date = end_date;
    rental.actual_return_date = std::nullopt;
    rental.total_cost = total_cost;
    rental.status = domain::RentalStatus::active;
    rental.created_at = std::chrono::system_clock::now();
    
    if (!storage.CreateRental(rental)) {
        return {
            RentalErrorCode::CONFLICT,
            "Failed to create rental",
            std::nullopt
        };
    }
    
    storage.UpdateCarAvailability(car_id, false);
    
    return {RentalErrorCode::OK, "", rental};
}

RentalResult RentalService::GetRentalById(const std::string& id) {
    auto& storage = storage::InMemoryStorage::Instance();
    auto rental = storage.GetRentalById(id);
    
    if (!rental.has_value()) {
        return {
            RentalErrorCode::NOT_FOUND,
            "Rental not found",
            std::nullopt
        };
    }
    
    return {RentalErrorCode::OK, "", rental};
}

RentalListResult RentalService::GetActiveRentalsByUserId(const std::string& user_id) {
    auto& storage = storage::InMemoryStorage::Instance();
    
    if (!storage.UserExists(user_id)) {
        return {
            RentalErrorCode::NOT_FOUND,
            "User not found",
            {},
            0
        };
    }
    
    auto rentals = storage.GetActiveRentalsByUserId(user_id);
    return {
        RentalErrorCode::OK,
        "",
        rentals,
        static_cast<int>(rentals.size())
    };
}

RentalListResult RentalService::GetRentalHistoryByUserId(const std::string& user_id) {
    auto& storage = storage::InMemoryStorage::Instance();
    
    if (!storage.UserExists(user_id)) {
        return {
            RentalErrorCode::NOT_FOUND,
            "User not found",
            {},
            0
        };
    }
    
    auto rentals = storage.GetRentalHistoryByUserId(user_id);
    return {
        RentalErrorCode::OK,
        "",
        rentals,
        static_cast<int>(rentals.size())
    };
}

RentalResult RentalService::CompleteRental(const std::string& id) {
    auto& storage = storage::InMemoryStorage::Instance();
    
    auto rental = storage.GetRentalById(id);
    if (!rental.has_value()) {
        return {
            RentalErrorCode::NOT_FOUND,
            "Rental not found",
            std::nullopt
        };
    }
    
    if (rental->status != domain::RentalStatus::active) {
        return {
            RentalErrorCode::CONFLICT,
            "Rental is already completed or cancelled",
            std::nullopt
        };
    }
    
    if (!storage.CompleteRental(id)) {
        return {
            RentalErrorCode::CONFLICT,
            "Failed to complete rental",
            std::nullopt
        };
    }
    
    storage.UpdateCarAvailability(rental->car_id, true);
    
    rental->status = domain::RentalStatus::completed;
    rental->actual_return_date = std::chrono::system_clock::now();
    
    return {RentalErrorCode::OK, "", rental};
}

} // namespace car_rental::services