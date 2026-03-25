#include "get_available_cars.hpp"

#include <services/car_service/car_service.hpp>
#include <domain/car.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>
#include <userver/utils/datetime.hpp>

#include <optional>
#include <string_view>

namespace car_rental::components {

namespace {

constexpr int kDefaultLimit = 20;
constexpr int kMinLimit = 1;
constexpr int kMaxLimit = 100;
constexpr int kMinOffset = 0;

std::optional<domain::CarClass> ParseCarClass(const std::string& value) {
    if (value.empty()) {
        return std::nullopt;
    }
    
    static const std::unordered_map<std::string, domain::CarClass> kClassMap = {
        {"economy", domain::CarClass::economy},
        {"compact", domain::CarClass::compact},
        {"midsize", domain::CarClass::midsize},
        {"fullsize", domain::CarClass::fullsize},
        {"luxury", domain::CarClass::luxury},
        {"suv", domain::CarClass::suv},
        {"van", domain::CarClass::van}
    };
    
    auto it = kClassMap.find(value);
    return (it != kClassMap.end()) ? std::make_optional(it->second) : std::nullopt;
}

std::string CarClassToString(domain::CarClass car_class) {
    static const std::unordered_map<domain::CarClass, std::string> kClassMap = {
        {domain::CarClass::economy, "economy"},
        {domain::CarClass::compact, "compact"},
        {domain::CarClass::midsize, "midsize"},
        {domain::CarClass::fullsize, "fullsize"},
        {domain::CarClass::luxury, "luxury"},
        {domain::CarClass::suv, "suv"},
        {domain::CarClass::van, "van"}
    };
    
    auto it = kClassMap.find(car_class);
    return (it != kClassMap.end()) ? it->second : "economy";
}

userver::formats::json::Value BuildCarJson(const domain::Car& car) {
    userver::formats::json::ValueBuilder builder;
    
    builder["id"] = car.id;
    builder["vin"] = car.vin;
    builder["brand"] = car.brand;
    builder["model"] = car.model;
    builder["year"] = car.year;
    builder["car_class"] = CarClassToString(car.car_class);
    builder["license_plate"] = car.license_plate;
    builder["daily_rate"] = car.daily_rate;
    builder["available"] = car.available;
    builder["created_at"] = userver::utils::datetime::TimePointTz(car.created_at);
    
    return builder.ExtractValue();
}

userver::formats::json::Value BuildCarListJson(
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

userver::formats::json::Value BuildValidationErrorJson(
    const std::string& message,
    const std::string& field = ""
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "validation_error";
    builder["message"] = message;
    
    if (!field.empty()) {
        userver::formats::json::ValueBuilder details_builder;
        userver::formats::json::ValueBuilder detail_builder;
        detail_builder["field"] = field;
        detail_builder["error"] = "invalid_value";
        details_builder.PushBack(detail_builder.ExtractValue());
        builder["details"] = details_builder.ExtractValue();
    }
    
    return builder.ExtractValue();
}

userver::formats::json::Value BuildInternalErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "internal_error";
    builder["message"] = message;
    return builder.ExtractValue();
}

struct QueryIntResult {
    int value;
    std::string error;
    bool ok;
};

QueryIntResult ParseQueryInt(
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

} // anonymous namespace

GetAvailableCars::GetAvailableCars(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string GetAvailableCars::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    std::optional<domain::CarClass> car_class;
    std::string class_str = request.GetArg("car_class");
    if (!class_str.empty()) {
        car_class = ParseCarClass(class_str);
        if (!car_class.has_value()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                BuildValidationErrorJson(
                    "Invalid car_class value",
                    "car_class"
                )
            );
        }
    }
    
    auto limit_result = ParseQueryInt(
        "limit",
        request.GetArg("limit"),
        kDefaultLimit,
        kMinLimit,
        kMaxLimit
    );
    
    if (!limit_result.ok) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            BuildValidationErrorJson(limit_result.error, "limit")
        );
    }
    
    auto offset_result = ParseQueryInt(
        "offset",
        request.GetArg("offset"),
        0,
        kMinOffset,
        std::numeric_limits<int>::max()
    );
    
    if (!offset_result.ok) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            BuildValidationErrorJson(offset_result.error, "offset")
        );
    }

    const auto result = services::CarService::GetAvailableCars(
        car_class,
        limit_result.value,
        offset_result.value
    );

    switch (result.code) {
        case services::CarErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(
                BuildCarListJson(result.cars, result.total)
            );

        case services::CarErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                BuildValidationErrorJson(result.message)
            );

        default:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kInternalServerError
            );
            return userver::formats::json::ToString(
                BuildInternalErrorJson("Unexpected error occurred")
            );
    }
}

} // namespace car_rental::components