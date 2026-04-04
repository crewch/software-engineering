#include "get_user_by_login.hpp"

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

namespace car_rental::components {

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
            utils::JsonBuilders::BuildValidationErrorJson("Login is required")
        );
    }

    const auto result = services::UserService::GetUserByLogin(login);

    switch (result.code) {
        case services::UserErrorCode::OK:
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildUserJson(*result.user)
            );

        case services::UserErrorCode::NOT_FOUND:
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            return userver::formats::json::ToString(
                utils::JsonBuilders::BuildNotFoundErrorJson(result.message)
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