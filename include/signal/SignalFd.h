#ifndef NETPP_SIGNALFD_H
#define NETPP_SIGNALFD_H

#include <mutex>

namespace netpp::handlers {
class SignalHandler;
}

namespace netpp::signal {
class SignalFd {
	friend class handlers::SignalHandler;
	friend class SignalWatcher;
public:
	~SignalFd() = default;

	void add(int sig);
	void del(int sig);
	bool watching(int sig);

private:
	SignalFd() noexcept;
	static SignalFd instance;
	static int signalFd;

	// 64 signals at max
	static volatile uint64_t m_watchingSignals;
};
}

#endif
