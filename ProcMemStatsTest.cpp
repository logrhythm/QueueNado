#include "ProcMemStatsTest.h"
#include "MockProcStats.h"


TEST_F(ProcMemStatsTest, MemStatsFromFile) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.1");
   procStats.SetPseudoSysConfTotalMemoryMB(15948); 
   ASSERT_TRUE(procStats.UpdateMemStats());

   EXPECT_EQ(10856,procStats.GetFreeMem());
   EXPECT_EQ(15948,procStats.GetTotalMem());
   EXPECT_EQ(4932,procStats.GetUsedMem());
   EXPECT_EQ(160,procStats.GetReservedMem());
   EXPECT_EQ(8039,procStats.GetTotalSwap());
   EXPECT_EQ(7254,procStats.GetFreeSwap());
   EXPECT_EQ(785,procStats.GetUsedSwap());
#endif
}

TEST_F(ProcMemStatsTest, MemStatsFromFile_FailedTotalMemReadingTriggerZeros) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.1_failed");
   procStats.SetPseudoSysConfTotalMemoryMB(15948); 
   ASSERT_FALSE(procStats.UpdateMemStats());

   EXPECT_EQ(0,procStats.GetFreeMem());
   EXPECT_EQ(0,procStats.GetTotalMem());
   EXPECT_EQ(0,procStats.GetUsedMem());
   EXPECT_EQ(0,procStats.GetReservedMem());
 
   // Next reading is correct
   procStats.SetMemFile("resources/meminfo.1");
   ASSERT_TRUE(procStats.UpdateMemStats());
   EXPECT_EQ(10856,procStats.GetFreeMem());
   EXPECT_EQ(15948,procStats.GetTotalMem());
   EXPECT_EQ(4932,procStats.GetUsedMem());
#endif
}

TEST_F(ProcMemStatsTest, MemStatsFromFile_FailedReservedMemoryReturnZero) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.2_failed");
   procStats.SetPseudoSysConfTotalMemoryMB(10); 
   ASSERT_TRUE(procStats.UpdateMemStats());

   EXPECT_EQ(10,procStats.GetTotalMem()); // failed but not caught
   EXPECT_EQ(10856,procStats.GetFreeMem());
   EXPECT_EQ(4932,procStats.GetUsedMem());
   EXPECT_EQ(0,procStats.GetReservedMem());  // total < free+used
#endif
}


TEST_F(ProcMemStatsTest, CompareSysConfWithMemFile) {
#ifdef LR_DEBUG
   ProcStats& procStats(ProcStats::Instance());
   procStats.ThreadRegister("TEST");

   procStats.Update();
   size_t total =  procStats.GetTotalMem();
   size_t totalMB =   procStats.ReadTotalMemMBOnce();
   ASSERT_EQ(total, totalMB);
#endif
}



TEST_F(ProcMemStatsTest, MemStatsFromFileNotExist) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/FILE_NOT_FOUND");
   ASSERT_FALSE(procStats.UpdateMemStats());

#endif
}
