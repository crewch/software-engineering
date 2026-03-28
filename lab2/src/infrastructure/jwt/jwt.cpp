#include "jwt.hpp"

#include <userver/logging/log.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace lab2::infrastructure {

namespace {
constexpr std::string_view kPrivateKey = "private_key";
constexpr std::string_view kIssuer = "issuer";
constexpr std::string_view kAudience = "audience";
}

JwtAuthComponent::JwtAuthComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {

    const auto private_key = config[kPrivateKey].As<std::string>();
    const auto issuer = config[kIssuer].As<std::string>();
    const auto audience = config[kAudience].As<std::string>();

    checker_ = std::make_shared<JwtAuthChecker>(
        private_key,
        issuer,
        audience
    );

    generator_ = std::make_shared<JwtTokenGenerator>(
        private_key,
        issuer,
        audience
    );
}

JwtAuthCheckerPtr JwtAuthComponent::GetChecker() const {
    return checker_;
}

std::shared_ptr<JwtTokenGenerator> JwtAuthComponent::GetGenerator() const {
    return generator_;
}

userver::yaml_config::Schema JwtAuthComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<
        userver::components::LoggableComponentBase>(R"(
type: object
description: JWT Auth Component (RS256)
additionalProperties: false
properties:
    private_key:
        type: string
        description: private key
    issuer:
        type: string
        description: Issuer for JWT tokens
    audience:
        type: string
        description: Audience for JWT tokens
required:
    - private_key
    - issuer
    - audience
)");
}

}  // namespace lab2::infrastructure
