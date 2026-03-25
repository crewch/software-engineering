#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>

#include <domain/user.hpp>
#include <domain/car.hpp>
#include <domain/rental.hpp>

namespace car_rental::storage {

class InMemoryStorage {
public:
    static InMemoryStorage& Instance();
   
    bool CreateUser(const domain::User& user);
    
    std::optional<domain::User> GetUserByLogin(const std::string& login);
    
    std::optional<domain::User> GetUserById(const std::string& id);
    
    std::vector<domain::User> SearchUsers(const std::string& name_mask);
    
    bool UserExists(const std::string& id);

    bool CreateCar(const domain::Car& car);
    
    std::vector<domain::Car> GetAllCars();
    
    std::vector<domain::Car> GetAvailableCars();
    
    std::vector<domain::Car> SearchCars(
        const std::optional<domain::CarClass>& car_class,
        bool available_only = true
    );
    
    std::optional<domain::Car> GetCarById(const std::string& id);
    
    std::optional<domain::Car> GetCarByVin(const std::string& vin);
    
    bool UpdateCarAvailability(const std::string& id, bool available);
    
    bool CarExists(const std::string& id);

    bool CreateRental(const domain::Rental& rental);
    
    std::optional<domain::Rental> GetRentalById(const std::string& id);
    
    std::vector<domain::Rental> GetActiveRentalsByUserId(const std::string& user_id);
    
    std::vector<domain::Rental> GetRentalHistoryByUserId(const std::string& user_id);
    
    std::vector<domain::Rental> GetAllRentalsByUserId(const std::string& user_id);
    
    bool CompleteRental(const std::string& id);
    
    bool IsCarAvailable(
        const std::string& car_id,
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end
    );

    void Clear();
    
    struct Stats {
        size_t user_count;
        size_t car_count;
        size_t rental_count;
    };
    Stats GetStats() const;

private:
    InMemoryStorage() = default;
    ~InMemoryStorage() = default;
    
    InMemoryStorage(const InMemoryStorage&) = delete;
    InMemoryStorage& operator=(const InMemoryStorage&) = delete;
    InMemoryStorage(InMemoryStorage&&) = delete;
    InMemoryStorage& operator=(InMemoryStorage&&) = delete;

    std::unordered_map<std::string, domain::User> users_;
    std::unordered_map<std::string, domain::User> users_by_id_;
    std::unordered_map<std::string, domain::Car> cars_;
    std::unordered_map<std::string, domain::Car> cars_by_vin_;
    std::unordered_map<std::string, domain::Rental> rentals_;
    
    std::unordered_multimap<domain::CarClass, std::string> cars_by_class_;
    std::unordered_multimap<std::string, std::string> rentals_by_user_;
    std::unordered_multimap<std::string, std::string> rentals_by_car_;

    mutable std::mutex mutex_;
};

} // namespace car_rental::storage