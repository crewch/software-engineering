#include "create_rental.hpp"

#include <services/rental_service/rental_service.hpp>
#include <domain/rental.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>
#include <userver/utils/datetime.hpp>

#include <string>
#include <chrono>

namespace car_rental::components {

namespace {

std::string RentalStatusToString(domain::RentalStatus status) {
    switch (status) {
        case domain::RentalStatus::active:
            return "active";
        case domain::RentalStatus::completed:
            return "completed";
        case domain::RentalStatus::cancelled:
            return "cancelled";
    }
    return "active";
}

userver::formats::json::Value BuildRentalJson(const domain::Rental& rental) {
    userver::formats::json::ValueBuilder builder;
    
    builder["id"] = rental.id;
    builder["user_id"] = rental.user_id;
    builder["car_id"] = rental.car_id;
    builder["start_date"] = userver::utils::datetime::TimePointTz(rental.start_date);
    builder["end_date"] = userver::utils::datetime::TimePointTz(rental.end_date);
    
    if (rental.actual_return_date.has_value()) {
        builder["actual_return_date"] = userver::utils::datetime::TimePointTz(
            rental.actual_return_date.value()
        );
    } else {
        builder["actual_return_date"] = nullptr;
    }
    
    builder["total_cost"] = rental.total_cost;
    builder["status"] = RentalStatusToString(rental.status);
    builder["created_at"] = userver::utils::datetime::TimePointTz(rental.created_at);
    
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

userver::formats::json::Value BuildNotFoundErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "not_found";
    builder["message"] = message;
    return builder.ExtractValue();
}

userver::formats::json::Value BuildCarNotAvailableErrorJson(
    const std::string& message,
    const std::string& car_id,
    const std::string& reason = "already_rented"
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "car_not_available";
    builder["message"] = message;
    builder["car_id"] = car_id;
    builder["reason"] = reason;
    return builder.ExtractValue();
}

userver::formats::json::Value BuildInternalErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "internal_error";
    builder["message"] = message;
    return builder.ExtractValue();
}

struct ParseDateResult {
    std::chrono::system_clock::time_point value;
    std::string error;
    bool ok;
};

ParseDateResult ParseISO8601(const std::string& date_str) {
    if (date_str.empty()) {
        return {
            std::chrono::system_clock::time_point{},
            "Date string is empty",
            false
        };
    }
    
    try {
        auto tp = userver::utils::datetime::UtcStringtime(
            date_str, 
            userver::utils::datetime::kRfc3339Format
        );
        return {tp, "", true};
    } catch (const std::exception& e) {
        return {
            std::chrono::system_clock::time_point{},
            fmt::format("Invalid date format: {}", e.what()),
            false
        };
    }
}

} // anonymous namespace

CreateRental::CreateRental(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string CreateRental::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    const auto& body = request.RequestBody();
    const auto json = userver::formats::json::FromString(body);

    std::string user_id = json["user_id"].As<std::string>();
    std::string car_id = json["car_id"].As<std::string>();
    std::string start_date_str = json["start_date"].As<std::string>();
    std::string end_date_str = json["end_date"].As<std::string>();

    auto start_result = ParseISO8601(start_date_str);
    if (!start_result.ok) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            BuildValidationErrorJson(start_result.error, "start_date")
        );
    }
    
    auto end_result = ParseISO8601(end_date_str);
    if (!end_result.ok) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            BuildValidationErrorJson(end_result.error, "end_date")
        );
    }

    const auto result = services::RentalService::CreateRental(
        user_id,
        car_id,
        start_result.value,
        end_result.value
    );

    switch (result.code) {
        case services::RentalErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(BuildRentalJson(*result.rental));

        case services::RentalErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                BuildValidationErrorJson(result.message)
            );

        case services::RentalErrorCode::NOT_FOUND:
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            return userver::formats::json::ToString(
                BuildNotFoundErrorJson(result.message)
            );

        case services::RentalErrorCode::CAR_NOT_AVAILABLE:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            return userver::formats::json::ToString(
                BuildCarNotAvailableErrorJson(result.message, car_id, "already_rented")
            );

        case services::RentalErrorCode::CONFLICT:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
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