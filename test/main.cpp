#include <gtest/gtest.h>
#include "MockSysCallEnvironment.h"

int main(int argc, char *argv[])
{
	testing::InitGoogleTest(&argc, argv);
	MockSysCallEnvironment *mock = new MockSysCallEnvironment;
	testing::AddGlobalTestEnvironment(mock);
    return RUN_ALL_TESTS();
}
