#ifndef NETPP_SOCKET_ENUMS_H
#define NETPP_SOCKET_ENUMS_H

namespace netpp::internal::socket {

/**
 * @brief a simplify tcp state
 * 
 */
enum class TcpState {
	Closed,			// at init state
	Listen,			// is listening
	Connecting,		// is connecting
	Established,	// connection established
	HalfClose			// is closing connection
};

#define SOCKET_OPT(XX) \
	XX(BROADCAST)	\
	XX(DEBUG)		\
	XX(DONTROUTE)	\
	XX(ERROR)		\
	XX(KEEPALIVE)	\
	XX(LINGER)		\
	XX(OOBINLINE)	\
	XX(RCVBUF)		\
	XX(SNDBUF)		\
	XX(RCVLOWAT)	\
	XX(SNDLOWAT)	\
	XX(RCVTIMEO)	\
	XX(SNDTIMEO)	\
	XX(REUSEADDR)	\
	XX(REUSEPORT)	\
	XX(TYPE)		\
	XX(USELOOPBACK)	\
	XX(MAXSEG)		\
	XX(NODELAY)		\
// TODO: support more options
// TODO: wrap set/get socket options

/**
 * @brief Supported socket options, not used yet
 * 
 */
enum class SocketOptions {
#define SOCKET_OPT_DEF(opt) E_##opt,
	SOCKET_OPT(SOCKET_OPT_DEF)
#undef SOCKET_OPT_DEF
};
// bool setSocketOption(SocketOptions);
// bool getSocketOption(SocketOptions);
}

#endif
