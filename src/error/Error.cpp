//
// Created by gaojian on 22-6-18.
//

#include "error/Error.h"

namespace netpp {
const std::string_view &getErrorDescription(Error error) noexcept
{
	static std::string_view a;
	return a;
}
}
