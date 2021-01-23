#ifndef NETPP_ERROR_CODE_H
#define NETPP_ERROR_CODE_H

#include <string>

namespace netpp::error {
enum class SocketError {
#ifndef SOCKET_ERROR_DEF
#define SOCKET_ERROR_DEF(error) E_##error,
#endif
#ifndef SOCKET_ERROR_NONE_LINUX
#define SOCKET_ERROR_NONE_LINUX(error) E_##error,
#endif
#ifndef LAST_SOCKET_ERROR_DEF
#define LAST_SOCKET_ERROR_DEF(error) E_##error
#endif
#include "SocketError.def"
};

std::string errorAsString(SocketError code);
SocketError getError(int code);
}

#endif
