//
// Created by gaojian on 22-6-18.
//

#include "error/Error.h"

namespace netpp {
const std::string_view &getErrorDescription(Error error) noexcept
{
	static constexpr std::string_view permissionDeniedString = "permission denied";
	static constexpr std::string_view notSupportString = "operation not support";
	static constexpr std::string_view addressInUseString = "address already in use";
	static constexpr std::string_view connectRefusedString = "connect refused";
	static constexpr std::string_view unreachableString = "host unreachable";
	static constexpr std::string_view timeoutString = "time out";
	static constexpr std::string_view badFDString = "bad file descriptor";
	static constexpr std::string_view fileDescriptorLimitedString = "file descriptor limited";
	static constexpr std::string_view memoryUnavailableString = "memory not enough";
	static constexpr std::string_view invalidAddressString = "invalid address";
	static constexpr std::string_view unknownString = "unknown error";
	static constexpr std::string_view inProgressString = "action in progress";
	static constexpr std::string_view brokenConnectionString = "connection broken";

	switch (error)
	{
		case Error::PermissionDenied:	return permissionDeniedString;
		case Error::NotSupport:			return notSupportString;
		case Error::AddressInUse:	return addressInUseString;
		case Error::ConnectRefused:	return connectRefusedString;
		case Error::Unreachable:	return unreachableString;
		case Error::Timeout:	return timeoutString;
		case Error::BadFD:	return badFDString;
		case Error::FileDescriptorLimited:	return fileDescriptorLimitedString;
		case Error::MemoryUnavailable:	return memoryUnavailableString;
		case Error::InvalidAddress:	return invalidAddressString;
		case Error::Unknown:	return unknownString;
		case Error::InProgress:	return inProgressString;
		case Error::BrokenConnection:	return brokenConnectionString;
	}

	static constexpr std::string_view empty;
	return empty;
}
}
