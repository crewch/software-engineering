#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <infrastructure/jwt/jwt_auth_factory.hpp>
#include <controllers/create_user/create_user.hpp>
#include <controllers/login/login.hpp>
#include <controllers/get_user_by_login/get_user_by_login.hpp>
#include <controllers/create_car/create_car.hpp>
#include <controllers/get_available_cars/get_available_cars.hpp>
#include <controllers/get_cars_by_class/get_cars_by_class.hpp>
#include <controllers/create_rental/create_rental.hpp>

int main(int argc, char* argv[]) {
  userver::server::handlers::auth::RegisterAuthCheckerFactory<lab2::infrastructure::JwtAuthCheckerFactory>();

  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::components::TestsuiteSupport>()
          .AppendComponentList(userver::clients::http::ComponentList())
          .Append<userver::clients::dns::Component>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::congestion_control::Component>()
          .Append<car_rental::components::CreateUser>()
          .Append<car_rental::components::Login>()
          .Append<car_rental::components::GetUserByLogin>()
          .Append<car_rental::components::CreateCar>()
          .Append<car_rental::components::GetAvailableCars>()
          .Append<car_rental::components::GetCarsByClass>()
          .Append<car_rental::components::CreateRental>()
          .Append<lab2::infrastructure::JwtAuthComponent>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}