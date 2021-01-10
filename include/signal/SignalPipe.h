#ifndef NETPP_SIGNALPIPE_H
#define NETPP_SIGNALPIPE_H

namespace netpp::handlers {
class SignalHandler;
}

namespace netpp::signal {
class SignalPipe {
	friend class handlers::SignalHandler;
public:
	~SignalPipe();

	static void handleSignal(int sig);

private:
	SignalPipe();
	inline static SignalPipe &instance()
	{
		static SignalPipe instance;
		return instance;
	}
	bool isPipeOpened();
	static int m_signalPipe[2];
};
}

#endif
