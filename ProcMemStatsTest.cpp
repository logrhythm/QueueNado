#include "ProcMemStatsTest.h"
#include "MockProcStats.h"
#include <iostream>
#include <thread>
#include <chrono>

TEST_F(ProcMemStatsTest, System_MemoryInfo) {
   MemoryInfo info("/proc/meminfo");
   EXPECT_EQ(info.ReadTotalMemMBOnce(), info.GetTotal());
   EXPECT_NE(0, info.GetTotal());
}

// Ref: For values check meminfo.1
// BuffersAndCache = Buffers+Cache
// Used = MemTotal-MemFree-BuffersAndCache
// Free = MemFree+BuffersAndCache
namespace MemInfo1 {
   const int readTotalKB = 16331548;
   const int readFreeKB = 8622852;
   const int readUsedKB = readTotalKB - readFreeKB;
   const int readBufferKB = 448612;
   const int readCacheKB = 2065524;
   
   const int swapTotalMB = 8232952 >> KB_TO_MB_SHIFT;
   const int swapFreeMB = 7364716 >> KB_TO_MB_SHIFT;
   const int swapUsedMB = swapTotalMB - swapFreeMB;
   
   // the calculated values
   const int readBufferAndCacheKB = readBufferKB + readCacheKB;
   const int calculatedTotalMB = (readTotalKB >> KB_TO_MB_SHIFT);
   const int calculatedUsedMB = ((readUsedKB - readBufferAndCacheKB) >> KB_TO_MB_SHIFT);
   const int calculatedFreeMB = ((readFreeKB + readBufferAndCacheKB) >> KB_TO_MB_SHIFT);
}

TEST_F(ProcMemStatsTest, MemoryInfoWithUpdate) {
#ifdef LR_DEBUG
   GMockMemoryInfo info;
   using ::testing::Return;
   EXPECT_CALL(info, ReadTotalMemMBOnce())
           .Times(1) // // pseudo value from meminfo.1 that is checked in two separate ways
           .WillRepeatedly(Return(15948));

   EXPECT_EQ(0, info.GetTotal());
   EXPECT_EQ(0, info.GetFree());
   EXPECT_EQ(0, info.GetUsed());
   ASSERT_TRUE(info.Update("resources/meminfo.1"));

   EXPECT_EQ(15948, info.GetTotal());
   EXPECT_EQ(MemInfo1::calculatedTotalMB, info.GetTotal());
   EXPECT_EQ(MemInfo1::calculatedFreeMB, info.GetFree()); // MemInfo + buffersAndCache
   EXPECT_EQ(MemInfo1::calculatedUsedMB, info.GetUsed());

   EXPECT_EQ(8039, info.GetTotalSwap());
   EXPECT_EQ(7192, info.GetFreeSwap());
   EXPECT_EQ(8039-7192, info.GetUsedSwap());
#endif
}


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
   EXPECT_EQ(MemInfo1::calculatedTotalMB, procStats.GetTotalMemory());
   EXPECT_EQ(MemInfo1::calculatedFreeMB, procStats.GetFreeMemory()); // MemInfo + buffersAndCache
   EXPECT_EQ(MemInfo1::calculatedUsedMB, procStats.GetUsedMemory());

   EXPECT_EQ(8039, procStats.GetTotalSwap());
   EXPECT_EQ(7192, procStats.GetFreeSwap());
   EXPECT_EQ(8039-7192, procStats.GetUsedSwap());
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

   EXPECT_EQ(0, procStats.GetFreeMemory());
   EXPECT_EQ(0, procStats.GetTotalMemory());
   EXPECT_EQ(0, procStats.GetUsedMemory());


   // Next reading is correct
   procStats.SetMemFile("resources/meminfo.1");
   EXPECT_TRUE(procStats.UpdateMemStats()); // 2nd time gmock
   EXPECT_EQ(MemInfo1::calculatedTotalMB, procStats.GetTotalMemory());
   EXPECT_EQ(MemInfo1::calculatedFreeMB, procStats.GetFreeMemory()); // MemInfo + buffersAndCache
   EXPECT_EQ(MemInfo1::calculatedUsedMB, procStats.GetUsedMemory());
#endif
}

TEST_F(ProcMemStatsTest, MemStatsFromFile_FailedMemoryReturnZero) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.2_failed");
   using ::testing::Return;
   EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
           .WillOnce(Return(10));
   ASSERT_FALSE(procStats.UpdateMemStats());

   EXPECT_EQ(0, procStats.GetTotalMemory()); // failed AND caught
   EXPECT_EQ(0, procStats.GetFreeMemory());
   EXPECT_EQ(0, procStats.GetUsedMemory());
#endif
}


TEST_F(ProcMemStatsTest, MemStatsFromFile_FailedTotalMemoryReturnZero) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.1");
   using ::testing::Return;
   EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
           .WillOnce(Return(10))
           .WillOnce(Return(MemInfo1::calculatedTotalMB));
   ASSERT_FALSE(procStats.UpdateMemStats());
   EXPECT_EQ(0, procStats.GetTotalMemory()); // failed AND caught
   EXPECT_EQ(0, procStats.GetFreeMemory());
   EXPECT_EQ(0, procStats.GetUsedMemory());

   ASSERT_TRUE(procStats.UpdateMemStats()); // Re-read. This time mock gives correct values
   EXPECT_EQ(MemInfo1::calculatedTotalMB, procStats.GetTotalMemory());
   EXPECT_EQ(MemInfo1::calculatedFreeMB, procStats.GetFreeMemory()); // MemInfo + buffersAndCache
   EXPECT_EQ(MemInfo1::calculatedUsedMB, procStats.GetUsedMemory());
   
#endif
}

TEST_F(ProcMemStatsTest, MemStatsFromFile_FailedSwapReturnZero) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.3_failed");
   using ::testing::Return;
   EXPECT_CALL(procStats.mGMockMemoryInfo, ReadTotalMemMBOnce())
           .WillOnce(Return(10))
           .WillOnce(Return(MemInfo1::calculatedTotalMB));
   ASSERT_FALSE(procStats.UpdateMemStats());
   EXPECT_EQ(0, procStats.GetTotalMemory()); // failed AND caught
   EXPECT_EQ(0, procStats.GetFreeMemory());
   EXPECT_EQ(0, procStats.GetUsedMemory());
   
   EXPECT_EQ(0, procStats.GetTotalSwap());
   EXPECT_EQ(0, procStats.GetUsedSwap());
   EXPECT_EQ(0, procStats.GetFreeSwap());

   procStats.SetMemFile("resources/meminfo.1");
   ASSERT_TRUE(procStats.UpdateMemStats()); // Re-read. This time mock gives correct values
   EXPECT_EQ(MemInfo1::calculatedTotalMB, procStats.GetTotalMemory());
   EXPECT_EQ(MemInfo1::calculatedFreeMB, procStats.GetFreeMemory()); // MemInfo + buffersAndCache
   EXPECT_EQ(MemInfo1::calculatedUsedMB, procStats.GetUsedMemory());
   
   EXPECT_EQ(MemInfo1::swapTotalMB, procStats.GetTotalSwap());
   EXPECT_EQ(MemInfo1::swapUsedMB, procStats.GetUsedSwap());
   EXPECT_EQ(MemInfo1::swapFreeMB, procStats.GetFreeSwap());
   
#endif
}




TEST(ProcMemStatsSystemTest, System_CompareSysConfWithMemFile) {
#ifdef LR_DEBUG
   ProcStats & procStats(ProcStats::Instance());
   procStats.ThreadRegister("TEST");
   MemoryInfo info("/proc/meminfo");

   procStats.Update();
   size_t total = procStats.GetTotalMemory();
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

// Compare this to the linux command 'free -m'
// 2nd line for memory (+/- buffers/cache) for used and free
//
// for example you can compare it from the output of: 
//  while :; do echo 'Hit CTRL+C'; free -m; sleep 3; done
TEST_F(ProcMemStatsTest, DISABLED_System_MemStatsFromFileNotExist) {
#ifdef LR_DEBUG
   MemoryInfo info("/proc/meminfo");
   for(size_t count = 0; count < 20; ++count) {
      ASSERT_TRUE(info.Update("/proc/meminfo"));
      std::cout << "\nTotal: " << info.GetTotal() 
              << "\tUsed: " << info.GetUsed() 
              << "\tFree: " << info.GetFree() << std::endl;
      std::cout << "Swap\tTotal: " << info.GetTotalSwap() 
              << "\tUsed: " << info.GetUsedSwap()
              << "\tFree: " << info.GetFreeSwap() << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(3));
   }
   
#endif
}

