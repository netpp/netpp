#ifndef TEST_SIGNAL_H
#define TEST_SIGNAL_H

struct SignalProcess {
public:
	static void prepareSignalProcess();
	static int pip[2];
	static int pid;
};

#endif
