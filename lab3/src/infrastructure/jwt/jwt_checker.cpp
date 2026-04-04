#include "jwt_checker.hpp"

#include <jwt-cpp/jwt.h>
#include <userver/http/common_headers.hpp>
#include <userver/logging/log.hpp>

namespace lab2::infrastructure {

namespace {
static constexpr std::string_view kBearerPrefix = "Bearer ";
}

JwtAuthChecker::JwtAuthChecker(const std::string& private_key,
                               std::string issuer,
                               std::string audience)
    : private_key_(private_key),
      issuer_(std::move(issuer)),
      audience_(std::move(audience)) {}

JwtAuthChecker::AuthCheckResult JwtAuthChecker::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {

    const auto auth_header =
        request.GetHeader(userver::http::headers::kAuthorization);

    if (auth_header.empty()) {
        return {AuthCheckResult::Status::kTokenNotFound,
                "Missing Authorization header"};
    }
    if (!auth_header.starts_with(kBearerPrefix)) {
        return {AuthCheckResult::Status::kInvalidToken,
                "Expected Bearer token"};
    }
    const std::string token =
        std::string(auth_header.substr(kBearerPrefix.size()));

    try {
        auto decoded = ::jwt::decode(token);

        auto verifier = ::jwt::verify()
            .allow_algorithm(::jwt::algorithm::hs256(private_key_))
            .with_issuer(issuer_)
            .with_audience(audience_)
            .leeway(30UL);
        verifier.verify(decoded);

        if (!decoded.has_subject()) {
            return {AuthCheckResult::Status::kInvalidToken,
                    "Missing subject claim"};
        }
        context.SetData("user_id", decoded.get_subject());

        return {AuthCheckResult::Status::kOk, ""};
    } catch (const ::jwt::error::token_verification_exception& e) {
        return {AuthCheckResult::Status::kInvalidToken,
                std::string("Verification failed: ") + e.what()};
    } catch (const std::exception& e) {
        return {AuthCheckResult::Status::kForbidden,
                std::string("Token processing error: ") + e.what()};
    }
}

}
