//
// Created by gaojian on 22-6-18.
//

#ifndef NETPP_ERROR_H
#define NETPP_ERROR_H

#include <string_view>

namespace netpp {
enum class Error {
	PermissionDenied,
	NotSupport,
	AddressInUse,
	ConnectRefused,
	Unreachable,
	Timeout,
	BadFD,
	FileDescriptorLimited,
	MemoryUnavailable,
	InvalidAddress,
	Unknown,
	InProgress,
	BrokenConnection,
};
extern const std::string_view &getErrorDescription(Error error) noexcept;
}

#endif //NETPP_ERROR_H
