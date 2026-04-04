#include "json_builders.hpp"

namespace car_rental::utils {

userver::formats::json::Value JsonBuilders::BuildUserJson(
    const domain::User& user
) {
    userver::formats::json::ValueBuilder builder;
    
    builder["id"] = user.GetId();
    builder["login"] = user.GetLogin();
    builder["first_name"] = user.GetFirstName();
    builder["last_name"] = user.GetLastName();
    builder["email"] = user.GetEmail();
    
    if (user.GetPhone().has_value()) {
        builder["phone"] = user.GetPhone().value();
    } else {
        builder["phone"] = nullptr;
    }
    
    builder["created_at"] = userver::utils::datetime::TimePointTz(
        user.GetCreatedAt()
    );
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildAuthJson(
    const std::string token
) {
    userver::formats::json::ValueBuilder builder;
    
    builder["token"] = token;
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildCarJson(
    const domain::Car& car
) {
    userver::formats::json::ValueBuilder builder;
    
    builder["id"] = car.GetId();
    builder["vin"] = car.GetVin();
    builder["brand"] = car.GetBrand();
    builder["model"] = car.GetModel();
    builder["year"] = car.GetYear();
    builder["car_class"] = domain::Car::CarClassToString(car.GetCarClass());
    builder["license_plate"] = car.GetLicensePlate();
    builder["daily_rate"] = car.GetDailyRate();
    builder["available"] = car.IsAvailable();
    builder["created_at"] = car.GetCreatedAt();
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildCarListJson(
    const std::vector<domain::Car>& cars,
    int total
) {
    userver::formats::json::ValueBuilder builder;
    
    userver::formats::json::ValueBuilder items_builder;
    for (const auto& car : cars) {
        items_builder.PushBack(BuildCarJson(car));
    }
    
    builder["items"] = items_builder.ExtractValue();
    builder["total"] = total;
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildRentalJson(
    const domain::Rental& rental
) {
    userver::formats::json::ValueBuilder builder;
    
    builder["id"] = rental.GetId();
    builder["user_id"] = rental.GetUserId();
    builder["car_id"] = rental.GetCarId();
    builder["start_date"] = rental.GetStartDate();
    builder["end_date"] = rental.GetEndDate();
    builder["total_cost"] = rental.GetTotalCost();
    builder["status"] = domain::Rental::RentalStatusToString(rental.GetStatus());
    builder["created_at"] = rental.GetCreatedAt();
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildValidationErrorJson(
    const std::string& message
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "validation_error";
    builder["message"] = message;
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildUnauthorizedErrorJson(
    const std::string& message
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "unauthorized_error";
    builder["message"] = message;
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildConflictErrorJson(
    const std::string& message
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "conflict";
    builder["message"] = message;
    
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildNotFoundErrorJson(
    const std::string& message
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "not_found";
    builder["message"] = message;
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildInternalErrorJson(
    const std::string& message
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "internal_error";
    builder["message"] = message;
    return builder.ExtractValue();
}

userver::formats::json::Value JsonBuilders::BuildCarNotAvailableErrorJson(
    const std::string& message,
    const std::string& reason
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "car_not_available";
    builder["message"] = message;
    builder["reason"] = reason;
    return builder.ExtractValue();
}

QueryIntResult JsonBuilders::ParseQueryInt(
    const std::string& param_name,
    const std::string& value,
    int default_value,
    int min_value,
    int max_value
) {
    if (value.empty()) {
        return {default_value, "", true};
    }
    
    try {
        int result = std::stoi(value);
        
        if (result < min_value) {
            return {
                0,
                fmt::format("{} must be >= {}", param_name, min_value),
                false
            };
        }
        
        if (result > max_value) {
            return {
                0,
                fmt::format("{} must be <= {}", param_name, max_value),
                false
            };
        }
        
        return {result, "", true};
        
    } catch (const std::invalid_argument&) {
        return {0, fmt::format("{} is not a valid integer", param_name), false};
    } catch (const std::out_of_range&) {
        return {0, fmt::format("{} is out of range", param_name), false};
    }
}

} // namespace car_rental::utils