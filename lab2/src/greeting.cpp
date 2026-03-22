#include <greeting.hpp>

#include <fmt/format.h>

#include <userver/utils/datetime.hpp>

namespace lab2::hello {

HelloResponseBody SayHelloTo(HelloRequestBody&& body) {
  return {.text = fmt::format("Hello to {} from Userver", std::move(body.name)),
          .current_time = userver::utils::datetime::TimePointTz{
              userver::utils::datetime::Now()}};
}

}