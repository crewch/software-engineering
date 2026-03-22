#pragma once

#include <docs/definitions/hello.hpp>

namespace lab2::hello {

HelloResponseBody SayHelloTo(HelloRequestBody&& body);

}