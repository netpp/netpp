#ifndef NETPP_ERROR_CODE_H
#define NETPP_ERROR_CODE_H

#include <string>

namespace netpp::error {
/**
 * @brief enumerate socket errors
 * 
 */
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

/// @brief Convert enumerated error to string
std::string errorAsString(SocketError code);
/// @brief get enumerated error
SocketError getError(int code);
}

#endif
