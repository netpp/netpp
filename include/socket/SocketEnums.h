#ifndef NETPP_SOCKET_ENUMS_H
#define NETPP_SOCKET_ENUMS_H

namespace netpp::socket {

enum class TcpState {
	Connecting,
	Connected,
	Disconnecting,
	Disconnected
};

enum class SocketOptions {
#ifdef SOCKET_OPT_DEF
#undef SOCKET_OPT_DEF
#endif
#ifdef LAST_SOCKET_OPT_DEF
#undef LAST_SOCKET_OPT_DEF
#endif
#ifndef SOCKET_OPT_DEF
#define SOCKET_OPT_DEF(opt) E_##opt,
#endif
#ifndef LAST_SOCKET_OPT_DEF
#define LAST_SOCKET_OPT_DEF(opt) E_##opt
#endif
#include "SocketOption.def"
};
bool setSocketOption(SocketOptions);
bool getSocketOption(SocketOptions);
}

#endif
