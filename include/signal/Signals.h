#ifndef NETPP_SIGNAL_H
#define NETPP_SIGNAL_H

#include <string>

namespace netpp::signal {
enum class Signals {
#ifdef SIGNAL_DEF
#undef SIGNAL_DEF
#endif
#ifdef LAST_SIGNAL_DEF
#undef LAST_SIGNAL_DEF
#endif
#define SIGNAL_DEF(sig)	E_##sig,
#define LAST_SIGNAL_DEF(sig) E_##sig
#include "Signal.def"
};
int toLinuxSignal(Signals signal);
Signals toNetppSignal(int signal);
std::string signalAsString(int signal);
std::string signalAsString(Signals signal);
bool ignoreByDefault(int signal);
bool ignoreByDefault(Signals signal);
}

#endif