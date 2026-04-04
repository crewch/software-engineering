#pragma once

#include <string>

namespace exceptions::domain {

struct ValidationError {
	std::string field;
	std::string message;
};

}