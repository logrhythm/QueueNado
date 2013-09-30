#include <gtest/gtest.h>
#include <g2log.hpp>
#include "include/global.h"
#include "TCMallocStats.h"
#include "MockSendStats.h"
#include "SendStats.h"
#include <google/malloc_extension.h>
#include <string>
#include <iostream> 
#include <vector>
#include <future>
#include <thread>


#ifndef MB_TO_B_SHIFT
#define MB_TO_B_SHIFT B_TO_MB_SHIFT
#endif 


// Disabled since for unit test the TCMALLOC_....REPORT_THRESHOLD is usually not set
// You can run this test by executing:
//
//  sudo TCMALLOC_RELEASE_RATE=10 TCMALLOC_LARGE_ALLOC_REPORT_THRESHOLD=16106127360 ./ProcStatsTest  --gtest_also_run_disabled_tests --gtest_filter="*TCMalloc*" 
//
TEST(TCMallocStatsTest, DISABLED_GetEnvReportThreshold) {
#ifdef LR_DEBUG
   TCMallocStats allocationStats;
   auto threshold = allocationStats.AllocReportThreshold();
   auto releaseRate = allocationStats.ReleaseRate();
   EXPECT_EQ(threshold, 16106127360);
   EXPECT_EQ(releaseRate, 10);
#endif
}

TEST(TCMallocStatsTest, GetHeapAndAllocate) {
#ifdef LR_DEBUG
   TCMallocStats stats;
   auto preHeapSizeMB = (stats.HeapSizeInBytes() >> B_TO_MB_SHIFT);
   auto preAllocatedMB = (stats.CurrentAllocatedBytes() >> B_TO_MB_SHIFT);
   auto preMemUsageMB = (stats.MemoryUsageLinuxReport() >> B_TO_MB_SHIFT);

   // Allocate 10 GB --> 10240MB
   const size_t MegaBitesIncrease = 10240;
   size_t bytesInMB = (MegaBitesIncrease << MB_TO_B_SHIFT);
   ASSERT_EQ(bytesInMB, 10737418240);

   std::string heapIncrease(bytesInMB, 'a');
   auto postHeapSizeMB = (stats.HeapSizeInBytes() >> B_TO_MB_SHIFT);
   auto postAllocatedMB = (stats.CurrentAllocatedBytes() >> B_TO_MB_SHIFT);
   auto postMemUsageMB = (stats.MemoryUsageLinuxReport() >> B_TO_MB_SHIFT);

   EXPECT_TRUE(postAllocatedMB >= MegaBitesIncrease) << " postAllocatedMB: " << postAllocatedMB;
   EXPECT_TRUE(postAllocatedMB >= (MegaBitesIncrease + preAllocatedMB))
           << " postAllocated: " << postAllocatedMB << " .preAllocatedMB: " << preAllocatedMB;

   EXPECT_TRUE(postHeapSizeMB >= MegaBitesIncrease) << " postHeapSizeMB: " << postHeapSizeMB;
   EXPECT_TRUE(postHeapSizeMB >= (MegaBitesIncrease + preHeapSizeMB))
           << " postHeapSizeMB: " << postHeapSizeMB << " .preHeapSizeMB: " << preHeapSizeMB;

   EXPECT_TRUE(postMemUsageMB >= MegaBitesIncrease) << " postMemUsageMB: " << postMemUsageMB;
   EXPECT_TRUE(postMemUsageMB >= (MegaBitesIncrease + preMemUsageMB))
           << " postMemUsageMB: " << postMemUsageMB << " .preMemUsageMB: " << preMemUsageMB;
#endif
}

TEST(TCMallocStatsTest, GetStatus) {
#ifdef LR_DEBUG
   TCMallocStats stats;
   auto releaseRate = stats.ReleaseRate();
   //auto allocReport = stats.AllocReportThreshold(); -- see disabled test

   auto currentBytes = stats.CurrentAllocatedBytes();
   auto heapSize = stats.HeapSizeInBytes();
   auto pageHeapFreeBytes = stats.PageHeapFreeBytes();
   auto pageHeapUnmappedBytes = stats.PageHeapUnmappedBytes();
   auto slackBytes = stats.SlackBytes();
   auto maxTotalThreadCacheBytes = stats.MaxTotalThreadCacheBytes();
   auto currentTotalThreadCacheBytes = stats.CurrentTotalThreadCacheBytes();

   size_t smallArbitrary = 1000; // bytes
   EXPECT_GT(releaseRate, 0); // default 1: see disabled test environment variable not set for unit tests
   //EXPECT_GT(allocReport, 0);  // see disabled  env variable not set in test
   EXPECT_GT(maxTotalThreadCacheBytes, smallArbitrary);

   EXPECT_GT(currentBytes, smallArbitrary);
   EXPECT_GT(heapSize, smallArbitrary);
   EXPECT_GT(pageHeapFreeBytes, smallArbitrary);
   //EXPECT_GT(pageHeapUnmappedBytes, 0); // likely zero but not guaranteed 
   EXPECT_GT(slackBytes, smallArbitrary);
   EXPECT_GT(currentTotalThreadCacheBytes, smallArbitrary);

   stats.SendToLogAllStats("TCMallocStatsTest"); // just for visual comparison
#endif
}

TEST(TCMallocStatsTest, SendStats) {
#ifdef LR_DEBUG
   MockSendStats statForwarder;
   TCMallocStats stats;
   auto preAlloc = (stats.CurrentAllocatedBytes() >> B_TO_KB_SHIFT);
   auto preHeap = (stats.HeapSizeInBytes() >> B_TO_KB_SHIFT);
   auto preMemUsage = (stats.MemoryUsageLinuxReport() >> B_TO_MB_SHIFT);

   stats.ForwardStats(statForwarder,{"TCMallocStatsTest"});

   const auto& results = statForwarder.mSendStatValues;
   const auto& keys = statForwarder.mSendStatKeys;
   const size_t TcMallocStatAmount = 5; // number of stats for TCMalloc
   ASSERT_EQ(results.size(), TcMallocStatAmount);
   ASSERT_EQ(keys.size(), TcMallocStatAmount);
   EXPECT_EQ(keys[0], "TCMallocStatsTest_Allocated");
   EXPECT_EQ(keys[1], "TCMallocStatsTest_Heap_Size");
   EXPECT_EQ(keys[2], "TCMallocStatsTest_Small_Object_Thread_Cache");
   EXPECT_EQ(keys[3], "TCMallocStatsTest_Memory_Slack");
   EXPECT_EQ(keys[4], "TCMallocStatsTest_Process_Mem_Usage");

   size_t smallArbitrary = 3; //KBytes
   EXPECT_GT(results[0], smallArbitrary);
   EXPECT_GT(results[1], smallArbitrary);
   EXPECT_GT(results[2], smallArbitrary);
   EXPECT_GT(results[3], smallArbitrary);
   EXPECT_GT(results[4], smallArbitrary);

   // measurements pre should be smaller or equal (technically slack, cache could get smaller)
   EXPECT_TRUE(results[0] >= preAlloc);
   EXPECT_TRUE(results[1] >= preHeap);
   //EXPECT_TRUE(results[2] >= ...);// cache is not measured since it MIGHT decrease
   EXPECT_TRUE(results[3] >= preMemUsage);

   // measurements post should be greater or equal (technically slack, cache could get smaller)
   auto postCurrentKB = (stats.CurrentAllocatedBytes() >> B_TO_KB_SHIFT);
   auto postHeapInKB = (stats.HeapSizeInBytes() >> B_TO_KB_SHIFT);
   auto postMemUsageMB = (stats.MemoryUsageLinuxReport() >> B_TO_MB_SHIFT);

   
   // Usage might differ a little but in either case it should be no more than 10%
   EXPECT_GE(results[0], 0.9*postCurrentKB);
   EXPECT_LT(results[0], 1.1*postCurrentKB);
     
   
   EXPECT_GE(results[1], 0.9*postHeapInKB);
   EXPECT_LT(results[1], 1.1*postHeapInKB);
   
   // [2]cache is not measured since it MIGHT decrease beyond easy to calculate formula. 
   // [3]dito for slack
   EXPECT_GE(results[4], 0.9*postMemUsageMB);
   EXPECT_LT(results[4], 1.1*postMemUsageMB) << "\n\tresult: " << results[3] <<". post: " << postMemUsageMB;

#endif
}

TEST(TCMallocStatsTest, LinuxMemUsageReporting) {
#ifdef LR_DEBUG
   TCMallocStats stats;
   MallocExtension::instance()->ReleaseFreeMemory();
   auto getMemoryLamba= [&stats]{ return stats.MemoryUsageLinuxReport();};

   // Make sure no funny business is happening if calling it in another thread
   auto willCalculateMemUsage1 = std::async(std::launch::async, getMemoryLamba);
   auto preMemUsage = willCalculateMemUsage1.get();

   // Allocate 10 GB --> 10240MB 
   const size_t MegaBitesIncrease = 10240;
   size_t bytesIncrease= (MegaBitesIncrease << MB_TO_B_SHIFT);
   ASSERT_EQ(bytesIncrease, 10737418240);
   std::string heapIncrease(bytesIncrease, 'B');
   EXPECT_EQ(heapIncrease.size(), 10737418240);
   
   auto postMemUsage = stats.MemoryUsageLinuxReport();

   // size of memory usage MIGHT NOT INCREASE due to reuse of not yet release memory
   EXPECT_TRUE(postMemUsage >= bytesIncrease) << " postMemUsage: " << postMemUsage 
           << " .preMemUsage: " << preMemUsage;

#endif
}
