#ifndef NETPP_SIGNAL_WATCHER_H
#define NETPP_SIGNAL_WATCHER_H

#include "./Signals.h"
#include <memory>

namespace netpp::signal {
// class SignalSet;
class SignalWatcher {
public:
	SignalWatcher() = default;

	void watch(Signals signal);
	void restore(Signals signal);

private:
	bool setSignalAction(Signals signal, void(*action)(int));
	// static std::shared_ptr<SignalSet> m_watchingSignals;
};
}

#endif
