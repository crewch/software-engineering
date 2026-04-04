#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/component_config.hpp>

namespace car_rental::components {

class CreateRental final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "HandlerCreateRental";
    
    CreateRental(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );
    
    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context
    ) const override;
};

} // namespace car_rental::components