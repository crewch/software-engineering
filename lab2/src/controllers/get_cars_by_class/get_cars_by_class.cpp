#include "get_cars_by_class.hpp"

#include <services/car_service/car_service.hpp>
#include <lib/json_builders/json_builders.hpp>
#include <domain/car.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>

#include <limits>
#include <string_view>

namespace car_rental::components {

GetCarsByClass::GetCarsByClass(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string GetCarsByClass::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
        request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson
    );

    std::string class_str = request.GetPathArg("car_class");
    
    domain::CarClass  car_class;
    
    try {
        car_class = domain::Car::CarClassFromString(class_str);
    } catch (const userver::formats::json::Exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            car_rental::utils::JsonBuilders::BuildValidationErrorJson(
                e.what()
            )
        );
    }

    constexpr int kDefaultLimit = 20;
    constexpr int kMinLimit = 1;
    constexpr int kMaxLimit = 100;
    constexpr int kMinOffset = 0;
    
    auto limit_result = car_rental::utils::JsonBuilders::ParseQueryInt(
        "limit",
        request.GetArg("limit"),
        kDefaultLimit,
        kMinLimit,
        kMaxLimit
    );
    
    if (!limit_result.ok) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            car_rental::utils::JsonBuilders::BuildValidationErrorJson(limit_result.error)
        );
    }
    
    auto offset_result = car_rental::utils::JsonBuilders::ParseQueryInt(
        "offset",
        request.GetArg("offset"),
        0,
        kMinOffset,
        std::numeric_limits<int>::max()
    );
    
    if (!offset_result.ok) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            car_rental::utils::JsonBuilders::BuildValidationErrorJson(offset_result.error)
        );
    }

    const auto result = services::CarService::GetCarsByClass(
        car_class,
        limit_result.value,
        offset_result.value
    );

    switch (result.code) {
        case services::CarErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildCarListJson(result.cars, result.total)
            );

        case services::CarErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildValidationErrorJson(result.message)
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