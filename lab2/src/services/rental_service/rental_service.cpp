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
    const lab2::rental::CreateRentalRequest& dto
) {  
    std::string user_id = boost::uuids::to_string(dto.user_id);
    std::string car_id = boost::uuids::to_string(dto.car_id);

    auto rental_result = domain::Rental::Create(
        user_id,
        car_id,
        dto.start_date,
        dto.end_date
    );

    if (std::holds_alternative<exceptions::domain::ValidationError>(rental_result)) {
        const auto& error = std::get<exceptions::domain::ValidationError>(rental_result);

        return {
            RentalErrorCode::VALIDATION_ERROR,
            error.message,
            std::nullopt
        };
    }

    auto& rental = std::get<domain::Rental>(rental_result);

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
    
    if (!storage.IsCarAvailable(car_id, rental.GetStartDate(), rental.GetEndDate())) {
        return {
            RentalErrorCode::CAR_NOT_AVAILABLE,
            "Car is not available for selected period",
            std::nullopt
        };
    }
    
    const int days = CalculateDays(rental.GetStartDate(), rental.GetEndDate());
    const double total_cost = CalculateTotalCost(car->GetDailyRate(), days);
    rental.RecalculateCost(total_cost);
    
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
    
    if (rental->GetStatus() != domain::RentalStatus::active) {
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
    
    storage.UpdateCarAvailability(rental->GetCarId(), true);
    
    rental->SetStatus(domain::RentalStatus::completed); 
    
    return {RentalErrorCode::OK, "", rental};
}

} // namespace car_rental::services