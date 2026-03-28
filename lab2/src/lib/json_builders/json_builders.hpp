#pragma once

#include <domain/user.hpp>
#include <domain/car.hpp>
#include <domain/rental.hpp>

#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>

#include <string>

namespace car_rental::utils {

struct QueryIntResult {
    int value;
    std::string error;
    bool ok;
};

class JsonBuilders {
public:
    static userver::formats::json::Value BuildUserJson(const domain::User& user);

    static userver::formats::json::Value BuildAuthJson(const std::string token);

    static userver::formats::json::Value BuildCarJson(const domain::Car& car);

    static userver::formats::json::Value BuildCarListJson(
        const std::vector<domain::Car>& cars,
        int total
    );

    static userver::formats::json::Value BuildRentalJson(const domain::Rental& rental);
    
    static userver::formats::json::Value BuildValidationErrorJson(
        const std::string& message
    );
    
    static userver::formats::json::Value BuildConflictErrorJson(
        const std::string& message
    );

    static userver::formats::json::Value BuildUnauthorizedErrorJson(
        const std::string& message
    );
    
    static userver::formats::json::Value BuildNotFoundErrorJson(
        const std::string& message
    );
    
    static userver::formats::json::Value BuildInternalErrorJson(
        const std::string& message
    );

    static userver::formats::json::Value BuildCarNotAvailableErrorJson(
        const std::string& message,
        const std::string& reason = "already_rented"
    );

    static QueryIntResult ParseQueryInt(
        const std::string& param_name,
        const std::string& value,
        int default_value,
        int min_value,
        int max_value
    );
};

} // namespace car_rental::utils
