#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/component_config.hpp>

#include <infrastructure/jwt/jwt_generator.hpp>

namespace car_rental::components {

class Login final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "HandlerLogin";
    
    Login(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );
    
    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context
    ) const override;

private:
    std::shared_ptr<lab2::infrastructure::JwtTokenGenerator> jwt_token_generator;
};

} // namespace car_rental::components