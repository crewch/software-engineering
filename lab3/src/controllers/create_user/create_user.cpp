#include "create_user.hpp"

#include <services/user_service/user_service.hpp>
#include <lib/json_builders/json_builders.hpp>
#include <domain/user.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>
#include <userver/utils/datetime.hpp>

#include <string>
#include <optional>

#include <docs/definitions/user.hpp>

namespace car_rental::components {

CreateUser::CreateUser(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string CreateUser::HandleRequestThrow(
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

    lab2::user::CreateUserRequest create_user_dto;
    try {
        create_user_dto = request_json.As<lab2::user::CreateUserRequest>();
    } catch (const userver::formats::json::Exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            utils::JsonBuilders::BuildValidationErrorJson(
                "Validation error: " + std::string(e.what())
            )
        );
    }

    const auto result = services::UserService::CreateUser(
        create_user_dto
    );

    switch (result.code) {
        case services::UserErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildUserJson(*result.user)
            );

        case services::UserErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildValidationErrorJson(
                    result.message
                )
            );

        case services::UserErrorCode::CONFLICT:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            return userver::formats::json::ToString(
                 utils::JsonBuilders::BuildConflictErrorJson(
                    result.message
                )
            );

        default:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kInternalServerError
            );
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildInternalErrorJson(
                    "Unexpected error occurred"
                )
            );
    }
}

} // namespace car_rental::components