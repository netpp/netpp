#include <gtest/gtest.h>
#include "SignalTest.h"

int main(int argc, char *argv[])
{
	SignalProcess::prepareSignalProcess();
	testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
