#include "create_car.hpp"

#include <services/car_service/car_service.hpp>
#include <domain/car.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>

#include <lib/uuid_generator/uuid_generator.hpp>

namespace car_rental::components {

namespace {

domain::CarClass ParseCarClass(const std::string& value) {
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
    return (it != kClassMap.end()) ? it->second : domain::CarClass::economy;
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

userver::formats::json::Value BuildValidationErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "validation_error";
    builder["message"] = message;
    return builder.ExtractValue();
}

userver::formats::json::Value BuildConflictErrorJson(
    const std::string& message,
    const std::string& field
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "conflict";
    builder["message"] = message;
    builder["conflict_field"] = field;
    return builder.ExtractValue();
}

userver::formats::json::Value BuildInternalErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "internal_error";
    builder["message"] = message;
    return builder.ExtractValue();
}

} // anonymous namespace

CreateCar::CreateCar(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string CreateCar::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {

    const auto& body = request.RequestBody();
    const auto json = userver::formats::json::FromString(body);

    domain::Car car;
    car.id = generateUUID();
    car.vin = json["vin"].As<std::string>();
    car.brand = json["brand"].As<std::string>();
    car.model = json["model"].As<std::string>();
    car.year = json["year"].As<int>();
    car.car_class = ParseCarClass(json["car_class"].As<std::string>());
    car.license_plate = json["license_plate"].As<std::string>();
    car.daily_rate = json["daily_rate"].As<double>();
    car.available = true;
    car.created_at = std::chrono::system_clock::now();

    const auto result = services::CarService::CreateCar(car);

    switch (result.code) {
        case services::CarErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(BuildCarJson(*result.car));

        case services::CarErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                BuildValidationErrorJson(result.message)
            );

        case services::CarErrorCode::CONFLICT:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            return userver::formats::json::ToString(
                BuildConflictErrorJson(result.message, "vin")
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