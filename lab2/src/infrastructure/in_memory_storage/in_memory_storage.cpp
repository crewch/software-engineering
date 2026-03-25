#include "in_memory_storage.hpp"

namespace car_rental::storage {
InMemoryStorage& InMemoryStorage::Instance() {
    static InMemoryStorage instance;
    return instance;
}

bool InMemoryStorage::CreateUser(const domain::User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (users_.find(user.login) != users_.end()) {
        return false;
    }
    
    if (users_by_id_.find(user.id) != users_by_id_.end()) {
        return false;
    }

    users_[user.login] = user;
    users_by_id_[user.id] = user;
    
    return true;
}

std::optional<domain::User> InMemoryStorage::GetUserByLogin(const std::string& login) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_.find(login);
    return (it != users_.end()) ? std::make_optional(it->second) : std::nullopt;
}

std::optional<domain::User> InMemoryStorage::GetUserById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_by_id_.find(id);
    return (it != users_by_id_.end()) ? std::make_optional(it->second) : std::nullopt;
}

std::vector<domain::User> InMemoryStorage::SearchUsers(const std::string& name_mask) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::User> result;
    result.reserve(users_.size());
    
    const bool mask_empty = name_mask.empty();
    
    for (const auto& [login, user] : users_) {
        if (mask_empty ||
            user.first_name.find(name_mask) != std::string::npos ||
            user.last_name.find(name_mask) != std::string::npos) {
            result.push_back(user);
        }
    }
    
    return result;
}

bool InMemoryStorage::UserExists(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return users_by_id_.find(id) != users_by_id_.end();
}

bool InMemoryStorage::CreateCar(const domain::Car& car) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (cars_by_vin_.find(car.vin) != cars_by_vin_.end()) {
        return false;
    }
    
    if (cars_.find(car.id) != cars_.end()) {
        return false;
    }

    cars_[car.id] = car;
    cars_by_vin_[car.vin] = car;
    
    cars_by_class_.emplace(car.car_class, car.id);
    
    return true;
}

std::vector<domain::Car> InMemoryStorage::GetAllCars() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::Car> result;
    result.reserve(cars_.size());
    
    for (const auto& [id, car] : cars_) {
        result.push_back(car);
    }
    
    return result;
}

std::vector<domain::Car> InMemoryStorage::GetAvailableCars() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::Car> result;
    result.reserve(cars_.size());
    
    for (const auto& [id, car] : cars_) {
        if (car.available) {
            result.push_back(car);
        }
    }
    
    return result;
}

std::vector<domain::Car> InMemoryStorage::SearchCars(
    const std::optional<domain::CarClass>& car_class,
    bool available_only
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::Car> result;
    
    if (car_class.has_value()) {
        auto range = cars_by_class_.equal_range(car_class.value());
        
        for (auto it = range.first; it != range.second; ++it) {
            auto car_it = cars_.find(it->second);
            if (car_it != cars_.end()) {
                if (!available_only || car_it->second.available) {
                    result.push_back(car_it->second);
                }
            }
        }
    } else {
        for (const auto& [id, car] : cars_) {
            if (!available_only || car.available) {
                result.push_back(car);
            }
        }
    }
    
    return result;
}

std::optional<domain::Car> InMemoryStorage::GetCarById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cars_.find(id);
    return (it != cars_.end()) ? std::make_optional(it->second) : std::nullopt;
}

std::optional<domain::Car> InMemoryStorage::GetCarByVin(const std::string& vin) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cars_by_vin_.find(vin);
    return (it != cars_by_vin_.end()) ? std::make_optional(it->second) : std::nullopt;
}

bool InMemoryStorage::UpdateCarAvailability(const std::string& id, bool available) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cars_.find(id);
    if (it == cars_.end()) {
        return false;
    }
    
    it->second.available = available;
    
    auto vin_it = cars_by_vin_.find(it->second.vin);
    if (vin_it != cars_by_vin_.end()) {
        vin_it->second.available = available;
    }
    
    return true;
}

bool InMemoryStorage::CarExists(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return cars_.find(id) != cars_.end();
}

bool InMemoryStorage::CreateRental(const domain::Rental& rental) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (rentals_.find(rental.id) != rentals_.end()) {
        return false;
    }

    rentals_[rental.id] = rental;
    
    rentals_by_user_.emplace(rental.user_id, rental.id);
    rentals_by_car_.emplace(rental.car_id, rental.id);
    
    return true;
}

std::optional<domain::Rental> InMemoryStorage::GetRentalById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = rentals_.find(id);
    return (it != rentals_.end()) ? std::make_optional(it->second) : std::nullopt;
}

std::vector<domain::Rental> InMemoryStorage::GetActiveRentalsByUserId(
    const std::string& user_id
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::Rental> result;
    
    auto range = rentals_by_user_.equal_range(user_id);
    for (auto it = range.first; it != range.second; ++it) {
        auto rental_it = rentals_.find(it->second);
        if (rental_it != rentals_.end() &&
            rental_it->second.status == domain::RentalStatus::active) {
            result.push_back(rental_it->second);
        }
    }
    
    return result;
}

std::vector<domain::Rental> InMemoryStorage::GetRentalHistoryByUserId(
    const std::string& user_id
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::Rental> result;
    
    auto range = rentals_by_user_.equal_range(user_id);
    for (auto it = range.first; it != range.second; ++it) {
        auto rental_it = rentals_.find(it->second);
        if (rental_it != rentals_.end() &&
            rental_it->second.status == domain::RentalStatus::completed) {
            result.push_back(rental_it->second);
        }
    }
    
    return result;
}

std::vector<domain::Rental> InMemoryStorage::GetAllRentalsByUserId(
    const std::string& user_id
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::Rental> result;
    
    auto range = rentals_by_user_.equal_range(user_id);
    for (auto it = range.first; it != range.second; ++it) {
        auto rental_it = rentals_.find(it->second);
        if (rental_it != rentals_.end()) {
            result.push_back(rental_it->second);
        }
    }
    
    return result;
}

bool InMemoryStorage::CompleteRental(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = rentals_.find(id);
    if (it == rentals_.end()) {
        return false;
    }
    
    if (it->second.status != domain::RentalStatus::active) {
        return false;
    }
    
    it->second.status = domain::RentalStatus::completed;
    it->second.end_date = std::chrono::system_clock::now();
    
    return true;
}

bool InMemoryStorage::IsCarAvailable(
    const std::string& car_id,
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto range = rentals_by_car_.equal_range(car_id);
    
    for (auto it = range.first; it != range.second; ++it) {
        auto rental_it = rentals_.find(it->second);
        if (rental_it != rentals_.end()) {
            const auto& rental = rental_it->second;
            
            if (rental.status != domain::RentalStatus::active) {
                continue;
            }
            
            if (start < rental.end_date && end > rental.start_date) {
                return false;
            }
        }
    }
    
    return true;
}

void InMemoryStorage::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    users_.clear();
    users_by_id_.clear();
    cars_.clear();
    cars_by_vin_.clear();
    cars_by_class_.clear();
    rentals_.clear();
    rentals_by_user_.clear();
    rentals_by_car_.clear();
}

InMemoryStorage::Stats InMemoryStorage::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    return Stats{
        .user_count = users_.size(),
        .car_count = cars_.size(),
        .rental_count = rentals_.size()
    };
}

} // namespace car_rental::storage