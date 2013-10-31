#include "ProcMemStatsTest.h"
#include "MockProcStats.h"

// This simple test verifieds that ReadTotalMemMBOnce is read the expected number of times
// it messed up the unit test earlier because if it failed to read the LOG printout would
// re-read the value throught the function call
TEST_F(ProcMemStatsTest, VerifyGMock_ExpectedCalls_ReadTotalMemMBOnce) {
#ifdef LR_DEBUG
   {
      MockProcStats procStats;
      procStats.SetMemFile("resources/meminfo.1");
      using ::testing::Return;
      EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
              .Times(1) // // pseudo value from meminfo.1 that is checked in two separate ways
              .WillRepeatedly(Return(15948));
      ASSERT_TRUE(procStats.UpdateMemStats()); // 1st time
   }
   {
      MockProcStats procStats;
      procStats.SetMemFile("resources/meminfo.1");
      using ::testing::Return;
      EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
              .Times(2) // // pseudo value from meminfo.1 that is checked in two separate ways
              .WillRepeatedly(Return(15948));
      ASSERT_TRUE(procStats.UpdateMemStats()); // 1st time
      procStats.SetMemFile("resources/meminfo.1_failed");
      ASSERT_FALSE(procStats.UpdateMemStats()); // 2nd time
   }
#endif
}

TEST_F(ProcMemStatsTest, MemStatsFromFile) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.1");
   using ::testing::Return;
   EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
           .Times(1) // // pseudo value from meminfo.1 that is checked in two separate ways
           .WillRepeatedly(Return(15948));

   ASSERT_TRUE(procStats.UpdateMemStats()); // 1st time
   EXPECT_EQ(15948, procStats.GetTotalMem());

   EXPECT_EQ(10856, procStats.GetFreeMem());
   EXPECT_EQ(4932, procStats.GetUsedMem());
   EXPECT_EQ(160, procStats.GetReservedMem());
   EXPECT_EQ(8039, procStats.GetTotalSwap());
   EXPECT_EQ(7254, procStats.GetFreeSwap());
   EXPECT_EQ(785, procStats.GetUsedSwap());
#endif
}

TEST_F(ProcMemStatsTest, MemStatsFromFile_FailedTotalMemReadingTriggerZeros) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.1_failed");
   using ::testing::Return;
   EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
           .WillRepeatedly(Return(15948)); // // pseudo value from meminfo.1 that is checked in two separate ways
   ASSERT_FALSE(procStats.UpdateMemStats()); // 1st time gmock

   EXPECT_EQ(0, procStats.GetFreeMem());
   EXPECT_EQ(0, procStats.GetTotalMem());
   EXPECT_EQ(0, procStats.GetUsedMem());
   EXPECT_EQ(0, procStats.GetReservedMem());

   // Next reading is correct
   procStats.SetMemFile("resources/meminfo.1");
   EXPECT_TRUE(procStats.UpdateMemStats()); // 2nd time gmock
   EXPECT_EQ(10856, procStats.GetFreeMem());
   EXPECT_EQ(15948, procStats.GetTotalMem());
   EXPECT_EQ(4932, procStats.GetUsedMem());
#endif
}

TEST_F(ProcMemStatsTest, MemStatsFromFile_FailedReservedMemoryReturnZero) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.2_failed");
   using ::testing::Return;
   EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
           .WillOnce(Return(10));
   ASSERT_TRUE(procStats.UpdateMemStats());

   EXPECT_EQ(10, procStats.GetTotalMem()); // failed but not caught
   EXPECT_EQ(10856, procStats.GetFreeMem());
   EXPECT_EQ(4932, procStats.GetUsedMem());
   EXPECT_EQ(0, procStats.GetReservedMem()); // total < free+used
#endif
}

TEST(ProcMemStatsSystemTest, System_CompareSysConfWithMemFile) {
#ifdef LR_DEBUG
   ProcStats & procStats(ProcStats::Instance());
   procStats.ThreadRegister("TEST");
   MemoryInfo info("/proc/meminfo");

   procStats.Update();
   size_t total = procStats.GetTotalMem();
   size_t totalMB = info.ReadTotalMemMBOnce();
   ASSERT_EQ(total, totalMB);
#endif
}

TEST_F(ProcMemStatsTest, System_MemStatsFromFileNotExist) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/FILE_NOT_FOUND");
   ASSERT_FALSE(procStats.UpdateMemStats());
#endif
}
