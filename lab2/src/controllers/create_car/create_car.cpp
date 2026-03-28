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
#include <lib/json_builders/json_builders.hpp>
#include <docs/definitions/car.hpp>

namespace car_rental::components {

CreateCar::CreateCar(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string CreateCar::HandleRequestThrow(
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
            car_rental::utils::JsonBuilders::BuildValidationErrorJson(
                "Invalid JSON format: " + std::string(e.what())
            )
        );
    }
    
    lab2::car::CreateCarRequest create_car_dto;
    try {
        create_car_dto = request_json.As<lab2::car::CreateCarRequest>();
    } catch (const userver::formats::json::Exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            car_rental::utils::JsonBuilders::BuildValidationErrorJson(
                "Validation error: " + std::string(e.what())
            )
        );
    }

    const auto result = services::CarService::CreateCar(create_car_dto);

    switch (result.code) {
        case services::CarErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(car_rental::utils::JsonBuilders::BuildCarJson(*result.car));

        case services::CarErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildValidationErrorJson(result.message)
            );

        case services::CarErrorCode::CONFLICT:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildConflictErrorJson(result.message)
            );

        default:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kInternalServerError
            );
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildInternalErrorJson("Unexpected error occurred")
            );
    }
}

} // namespace car_rental::components