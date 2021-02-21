#include "error/SocketError.h"
#include <cstring>
extern "C" {
#include <errno.h>
}

namespace netpp::error {
std::string errorAsString(SocketError code)
{
#ifdef SOCKET_ERROR_DEF
#undef SOCKET_ERROR_DEF
#endif
#ifdef SOCKET_ERROR_NONE_LINUX
#undef SOCKET_ERROR_NONE_LINUX
#endif
#ifdef LAST_SOCKET_ERROR_DEF
#undef LAST_SOCKET_ERROR_DEF

#endif
#ifndef SOCKET_ERROR_DEF
#define SOCKET_ERROR_DEF(error) case SocketError::E_##error: err = E##error; break;
#endif
#ifndef SOCKET_ERROR_NONE_LINUX
#define SOCKET_ERROR_NONE_LINUX(error) case SocketError::E_##error: break;
#endif
#ifndef LAST_SOCKET_ERROR_DEF
#define LAST_SOCKET_ERROR_DEF(error) SOCKET_ERROR_DEF(error)
#endif
	int err = -1;
	switch (code)
	{
#include "error/SocketError.def"
	}
	return std::strerror(err);
}

SocketError getError(int code)
{
#ifdef SOCKET_ERROR_DEF
#undef SOCKET_ERROR_DEF
#endif
#ifdef SOCKET_ERROR_NONE_LINUX
#undef SOCKET_ERROR_NONE_LINUX
#endif
#ifdef LAST_SOCKET_ERROR_DEF
#undef LAST_SOCKET_ERROR_DEF

#endif
#ifndef SOCKET_ERROR_DEF
#define SOCKET_ERROR_DEF(error) case E##error: err = SocketError::E_##error; break;
#endif
#ifndef SOCKET_ERROR_NONE_LINUX
#define SOCKET_ERROR_NONE_LINUX(error) ;
#endif
#ifndef LAST_SOCKET_ERROR_DEF
#define LAST_SOCKET_ERROR_DEF(error) SOCKET_ERROR_DEF(error)
#endif
	SocketError err = SocketError::E_UNKOWN;
	switch (code)
	{
#include "error/SocketError.def"
	}
	return err;
}
}
