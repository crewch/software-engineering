#pragma once

#include "jwt.hpp"

#include <userver/server/handlers/auth/auth_checker_factory.hpp>

namespace lab2::infrastructure {

class JwtAuthCheckerFactory final
    : public userver::server::handlers::auth::AuthCheckerFactoryBase {
public:
    static constexpr const char* kAuthType = "jwt-auth";

    explicit JwtAuthCheckerFactory(
        const userver::components::ComponentContext& context);

    userver::server::handlers::auth::AuthCheckerBasePtr
    MakeAuthChecker(
        const userver::server::handlers::auth::HandlerAuthConfig&) const override;

private:
    JwtAuthComponent& component_;
};

}  // namespace lab2::infrastructure
