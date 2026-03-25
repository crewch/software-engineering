#pragma once

#include <optional>
#include <vector>
#include <chrono>
#include <string>
#include <domain/rental.hpp>

namespace car_rental::services {

enum class RentalErrorCode {
    OK,
    VALIDATION_ERROR,
    NOT_FOUND,
    CONFLICT,
    CAR_NOT_AVAILABLE,
    INTERNAL_ERROR
};

struct RentalResult {
    RentalErrorCode code;
    std::string message;
    std::optional<domain::Rental> rental;
};

struct RentalListResult {
    RentalErrorCode code;
    std::string message;
    std::vector<domain::Rental> rentals;
    int total;
};

class RentalService {
public:
    static RentalResult CreateRental(
        const std::string& user_id,
        const std::string& car_id,
        std::chrono::system_clock::time_point start_date,
        std::chrono::system_clock::time_point end_date
    );
    
    static RentalResult GetRentalById(const std::string& id);
    
    static RentalListResult GetActiveRentalsByUserId(const std::string& user_id);
    
    static RentalListResult GetRentalHistoryByUserId(const std::string& user_id);
    
    static RentalResult CompleteRental(const std::string& id);
};

} // namespace car_rental::services