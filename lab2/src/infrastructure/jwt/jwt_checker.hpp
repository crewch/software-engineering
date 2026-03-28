#pragma once

#include <memory>
#include <string>

#include <userver/server/handlers/auth/auth_checker_base.hpp>

namespace lab2::infrastructure {

class JwtAuthChecker final
    : public userver::server::handlers::auth::AuthCheckerBase {
public:
    using AuthCheckResult =
        userver::server::handlers::auth::AuthCheckResult;

    JwtAuthChecker(const std::string& private_key,
                   std::string issuer,
                   std::string audience);

    AuthCheckResult CheckAuth(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override;

    bool SupportsUserAuth() const noexcept override {
        return true;
    }

private:
    std::string private_key_;
    std::string issuer_;
    std::string audience_;
};

using JwtAuthCheckerPtr = std::shared_ptr<JwtAuthChecker>;

}  // namespace lab2::infrastructure
