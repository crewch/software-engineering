#pragma once

#include "jwt_checker.hpp"
#include "jwt_generator.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/loggable_component_base.hpp>
#include <userver/yaml_config/schema.hpp>

#include <memory>

namespace lab2::infrastructure {

class JwtAuthComponent final: public userver::components::LoggableComponentBase {
public:
    static constexpr auto kName = "jwt-auth-component";

    JwtAuthComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

    JwtAuthCheckerPtr GetChecker() const;
    std::shared_ptr<JwtTokenGenerator> GetGenerator() const;

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    JwtAuthCheckerPtr checker_;
    std::shared_ptr<JwtTokenGenerator> generator_;
};

}  // namespace lab2::infrastructure
