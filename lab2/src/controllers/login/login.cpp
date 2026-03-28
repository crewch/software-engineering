#include "login.hpp"

#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>

#include <lib/json_builders/json_builders.hpp>
#include <docs/definitions/auth.hpp>
#include <infrastructure/jwt/jwt.hpp>
#include <services/user_service/user_service.hpp>

namespace car_rental::components {

Login::Login(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), 
    jwt_token_generator(context.FindComponent<lab2::infrastructure::JwtAuthComponent>().GetGenerator()) {}

std::string Login::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson);
    
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

    lab2::auth::LoginRequest login_dto;
    try {
        login_dto = request_json.As<lab2::auth::LoginRequest>();
    } catch (const userver::formats::json::Exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            car_rental::utils::JsonBuilders::BuildValidationErrorJson(
                "Validation error: " + std::string(e.what())
            )
        );
    }

    const auto result = services::UserService::Login(login_dto, jwt_token_generator);

    switch (result.code) {
        case services::AuthErrorCode::OK:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kOk
            );
            return userver::formats::json::ToString(car_rental::utils::JsonBuilders::BuildAuthJson(*result.token));

        case services::AuthErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kBadRequest
            );
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildValidationErrorJson(result.message)
            );

        case services::AuthErrorCode::INVALID_CREDENTIALS:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kUnauthorized
            );
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildUnauthorizedErrorJson(result.message)
            );
        
        case services::AuthErrorCode::NOT_FOUND:
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kNotFound
            );
            return userver::formats::json::ToString(
                car_rental::utils::JsonBuilders::BuildNotFoundErrorJson(result.message)
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