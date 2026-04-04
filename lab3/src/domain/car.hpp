#pragma once

#include <fmt/format.h>
#include <string>
#include <chrono>
#include <userver/formats/json/exception.hpp>
#include <variant>
#include <regex>
#include <lib/uuid_generator/uuid_generator.hpp>
#include <domain/exceptions.hpp>

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

class Car {
public:
    static std::variant<exceptions::domain::ValidationError, Car> Create(
        const std::string& vin,
        const std::string& brand,
        const std::string& model,
        int year,
        CarClass car_class,
        const std::string& license_plate,
        double daily_rate
    ) {
        if (!IsValidVin(vin)) {
            return exceptions::domain::ValidationError{
                "vin",
                "VIN must be 17 characters (A-H, J-N, P, R, S, T, V, Z, 0-9)"
            };
        }
        
        if (brand.empty() || brand.length() > kMaxBrandLength) {
            return exceptions::domain::ValidationError{
                "brand",
                fmt::format("Brand must be 1-{} characters", kMaxBrandLength)
            };
        }
        
        if (model.empty() || model.length() > kMaxModelLength) {
            return exceptions::domain::ValidationError{
                "model",
                fmt::format("Model must be 1-{} characters", kMaxModelLength)
            };
        }
        
        if (year < kMinYear || year > kMaxYear) {
            return exceptions::domain::ValidationError{
                "year",
                fmt::format("Year must be between {} and {}", kMinYear, kMaxYear)
            };
        }
        
        if (daily_rate < 0) {
            return exceptions::domain::ValidationError{
                "daily_rate",
                "Daily rate must be >= 0"
            };
        }
        
        Car car;
        car.id_ = generateUUID();
        car.vin_ = vin;
        car.brand_ = brand;
        car.model_ = model;
        car.year_ = year;
        car.car_class_ = car_class;
        car.license_plate_ = license_plate;
        car.daily_rate_ = daily_rate;
        car.available_ = true;
        car.created_at_ = std::chrono::system_clock::now();
        
        return car;
    }

    const std::string& GetId() const { return id_; }
    const std::string& GetVin() const { return vin_; }
    const std::string& GetBrand() const { return brand_; }
    const std::string& GetModel() const { return model_; }
    int GetYear() const { return year_; }
    CarClass GetCarClass() const { return car_class_; }
    const std::string& GetLicensePlate() const { return license_plate_; }
    double GetDailyRate() const { return daily_rate_; }
    bool IsAvailable() const { return available_; }
    const std::chrono::system_clock::time_point& GetCreatedAt() const { return created_at_; }

    void SetAvailable(bool available) { available_ = available; }

    static std::string CarClassToString(CarClass car_class) {
        switch (car_class) {
            case CarClass::economy: return "economy";
            case CarClass::compact: return "compact";
            case CarClass::midsize: return "midsize";
            case CarClass::fullsize: return "fullsize";
            case CarClass::luxury: return "luxury";
            case CarClass::suv: return "suv";
            case CarClass::van: return "van";
        }

        throw userver::formats::json::Exception("Unknown CarClass value");
    }

    static CarClass CarClassFromString(const std::string& str) {
        if (str == "economy") return CarClass::economy;
        if (str == "compact") return CarClass::compact;
        if (str == "midsize") return CarClass::midsize;
        if (str == "fullsize") return CarClass::fullsize;
        if (str == "luxury") return CarClass::luxury;
        if (str == "suv") return CarClass::suv;
        if (str == "van") return CarClass::van;
        
        throw userver::formats::json::Exception("Unknown CarClass name");
    }

private:
    std::string id_;
    std::string vin_;
    std::string brand_;
    std::string model_;
    int year_;
    CarClass car_class_;
    std::string license_plate_;
    double daily_rate_;
    bool available_;
    std::chrono::system_clock::time_point created_at_;

    static constexpr int kVinLength = 17;
    static constexpr int kMaxBrandLength = 50;
    static constexpr int kMaxModelLength = 50;
    static constexpr int kMinYear = 1900;
    static constexpr int kMaxYear = 2030;

    static bool IsValidVin(const std::string& vin) {
        if (vin.length() != kVinLength) {
            return false;
        }
        static const std::regex vin_pattern("^[A-HJ-NPR-Z0-9]{17}$");
        return std::regex_match(vin, vin_pattern);
    }
};

} // namespace car_rental::domain