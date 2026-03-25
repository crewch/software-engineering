#include "create_user.hpp"

#include <services/user_service/user_service.hpp>
#include <domain/user.hpp>

#include <userver/server/http/http_status.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>
#include <userver/utils/datetime.hpp>

#include <string>
#include <optional>

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

userver::formats::json::Value BuildConflictErrorJson(
    const std::string& message,
    const std::string& field = ""
) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "conflict";
    builder["message"] = message;
    
    if (!field.empty()) {
        builder["conflict_field"] = field;
    }
    
    return builder.ExtractValue();
}

userver::formats::json::Value BuildInternalErrorJson(const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["code"] = "internal_error";
    builder["message"] = message;
    return builder.ExtractValue();
}

} // anonymous namespace

CreateUser::CreateUser(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context) {}

std::string CreateUser::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    const auto& body = request.RequestBody();
    const auto json = userver::formats::json::FromString(body);

    std::string login = json["login"].As<std::string>();
    std::string first_name = json["first_name"].As<std::string>();
    std::string last_name = json["last_name"].As<std::string>();
    std::string email = json["email"].As<std::string>();
    std::string password = json["password"].As<std::string>();
    
    std::string phone;
    if (json.HasMember("phone") && !json["phone"].IsNull()) {
        phone = json["phone"].As<std::string>();
    }

    const auto result = services::UserService::CreateUser(
        login,
        first_name,
        last_name,
        email,
        phone,
        password
    );

    switch (result.code) {
        case services::UserErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(BuildUserJson(*result.user));

        case services::UserErrorCode::VALIDATION_ERROR:
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ToString(
                BuildValidationErrorJson(result.message)
            );

        case services::UserErrorCode::CONFLICT:
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            return userver::formats::json::ToString(
                BuildConflictErrorJson(result.message, "login")
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