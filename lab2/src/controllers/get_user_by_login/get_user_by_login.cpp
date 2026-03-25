#include "get_user_by_login.hpp"

#include <services/user_service/user_service.hpp>
#include <domain/user.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>
#include <userver/utils/datetime.hpp>

#include <string>

namespace car_rental::components {

namespace {

userver::formats::json::Value BuildUserJson(const domain::User& user) {
    userver::formats::json::ValueBuilder builder;
    
    builder["id"] = user.id;
    builder["login"] = user.login;
    builder["first_name"] = user.first_name;
    builder["last_name"] = user.last_name;
    builder["email"] = user.email;
    
    if (user.phone.has_value()) {
        builder["phone"] = user.phone.value();
    } else {
        builder["phone"] = nullptr;
    }
    
    builder["created_at"] = userver::utils::datetime::TimePointTz(user.created_at);
    
    return builder.ExtractValue();
}

userver::formats::json::Value BuildNotFoundErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "not_found";
    builder["message"] = message;
    return builder.ExtractValue();
}

userver::formats::json::Value BuildInternalErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "internal_error";
    builder["message"] = message;
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

} // anonymous namespace

GetUserByLogin::GetUserByLogin(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string GetUserByLogin::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {

    const std::string login = request.GetPathArg("login");
    
    if (login.empty()) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return userver::formats::json::ToString(
            BuildValidationErrorJson("Login is required")
        );
    }

    const auto result = services::UserService::GetUserByLogin(login);

    switch (result.code) {
        case services::UserErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(BuildUserJson(*result.user));

        case services::UserErrorCode::NOT_FOUND:
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            return userver::formats::json::ToString(
                BuildNotFoundErrorJson(result.message)
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