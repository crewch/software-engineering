#include "create_rental.hpp"

#include <services/rental_service/rental_service.hpp>
#include <lib/json_builders/json_builders.hpp>
#include <docs/definitions/rental.hpp>
#include <domain/rental.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>
#include <userver/utils/datetime.hpp>

#include <string>

namespace car_rental::components {

CreateRental::CreateRental(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string CreateRental::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson
    );

    userver::formats::json::Value request_json;
    try {
        request_json = userver::formats::json::FromString(request.RequestBody());
    } catch (const userver::formats::json::Exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            utils::JsonBuilders::BuildValidationErrorJson(
                "Invalid JSON format: " + std::string(e.what())
            )
        );
    }

    lab2::rental::CreateRentalRequest create_rental_dto;
    try {
        create_rental_dto = request_json.As<lab2::rental::CreateRentalRequest>();
    } catch (const userver::formats::json::Exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            utils::JsonBuilders::BuildValidationErrorJson(
                "Validation error: " + std::string(e.what())
            )
        );
    }

    const auto result = services::RentalService::CreateRental(
        create_rental_dto
    );

    switch (result.code) {
        case services::RentalErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(utils::JsonBuilders::BuildRentalJson(*result.rental));

        case services::RentalErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildValidationErrorJson(result.message)
            );

        case services::RentalErrorCode::NOT_FOUND:
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildNotFoundErrorJson(result.message)
            );

        case services::RentalErrorCode::CAR_NOT_AVAILABLE:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildCarNotAvailableErrorJson(result.message, "already_rented")
            );

        case services::RentalErrorCode::CONFLICT:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildValidationErrorJson(result.message)
            );

        default:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kInternalServerError
            );
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildInternalErrorJson("Unexpected error occurred")
            );
    }
}

} // namespace car_rental::components