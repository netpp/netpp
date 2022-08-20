#include <gtest/gtest.h>
#include "support/ThreadPool.hpp"
#include <vector>
#include <thread>

class ThreadPoolTest : public testing::Test {
public:
	inline void threadPoolSleepAdd(int i)
	{
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
		threadPoolSleepAddValue += i;
	}
protected:
	void SetUp() override {}
	void TearDown() override {}
	// static void SetUpTestCase() { threadPoolSleepAddValue = 0; }
	// static void TearDownTestCase() {}
	static std::atomic_int threadPoolSleepAddValue;
};
std::atomic_int ThreadPoolTest::threadPoolSleepAddValue = 0;

TEST_F(ThreadPoolTest, RunTaskTest)
{
	netpp::ThreadPool threadPool(1);
	threadPool.start();
	EXPECT_EQ(threadPool.queuedTask(), 0);
	threadPool.run(&ThreadPoolTest::threadPoolSleepAdd, this, 1);
	threadPool.run(&ThreadPoolTest::threadPoolSleepAdd, this, 2);
	threadPool.run(&ThreadPoolTest::threadPoolSleepAdd, this, 3);
	threadPool.run(&ThreadPoolTest::threadPoolSleepAdd, this, 4);
	threadPool.run(&ThreadPoolTest::threadPoolSleepAdd, this, 5);
	threadPool.run(&ThreadPoolTest::threadPoolSleepAdd, this, 6);
	threadPool.run(&ThreadPoolTest::threadPoolSleepAdd, this, 7);
	EXPECT_GT(threadPool.queuedTask(), 4);
	threadPool.waitForDone(0);
	EXPECT_EQ(ThreadPoolTest::threadPoolSleepAddValue, 28);
}
