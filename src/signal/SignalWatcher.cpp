#include "signal/SignalWatcher.h"
#include "signal/SignalFd.h"

namespace netpp::signal {
void SignalWatcher::watch(Signals signal)
{
	SignalFd::instance.add(toLinuxSignal(signal));
}

void SignalWatcher::restore(Signals signal)
{
	SignalFd::instance.del(toLinuxSignal(signal));
}
}
