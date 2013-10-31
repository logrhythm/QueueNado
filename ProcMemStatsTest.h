#include "gtest/gtest.h"
#include "ProcStats.h"
class ProcMemStatsTest : public ::testing::Test
{
public:
    ProcMemStatsTest() : mProcStats(ProcStats::Instance()) {};

protected:
	virtual void SetUp() {}
	virtual void TearDown() {}
	ProcStats& mProcStats;
};
