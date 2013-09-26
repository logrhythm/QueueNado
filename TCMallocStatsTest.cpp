#include <gtest/gtest.h>
#include <g2log.hpp>
#include <string>
#include <iostream> // for disabled test
#include <vector>

#include "TCMallocStats.h"
#include "MockSendStats.h"
#include "SendStats.h"

// Disabled since for unit test the TCMALLOC_....REPORT_THRESHOLD is usually not set
// You can run this test by executing:
//
//  sudo TCMALLOC_RELEASE_RATE=10 TCMALLOC_LARGE_ALLOC_REPORT_THRESHOLD=16106127360 ./ProcStatsTest  --gtest_also_run_disabled_tests --gtest_filter="*TCMalloc*" 
//

TEST(TCMallocStatsTest, DISABLED_GetEnvReportThreshold) {
   using namespace TCMallocStats;
   auto threshold = AllocReportThreshold();
   auto releaseRate = ReleaseRate();
   EXPECT_EQ(threshold, 16106127360);
   EXPECT_EQ(releaseRate, 10);
}

TEST(TCMallocStatsTest, GetStatus) {
   using namespace TCMallocStats;
   auto releaseRate = ReleaseRate();
   //auto allocReport = AllocReportThreshold(); -- see disabled test

   auto currentBytes = CurrentAllocatedBytes();
   auto heapSize = HeapSizeInBytes();
   auto pageHeapFreeBytes = PageHeapFreeBytes();
   auto pageHeapUnmappedBytes = PageHeapUnmappedBytes();
   auto slackBytes = SlackBytes();
   auto maxTotalThreadCacheBytes = MaxTotalThreadCacheBytes();
   auto currentTotalThreadCacheBytes = CurrentTotalThreadCacheBytes();

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

   SendToLogAllStats("TCMallocStatsTest"); // just for visual comparison
}

TEST(TCMallocStatsTest, SendStats) {
   MockSendStats statForwarder;
   using namespace TCMallocStats;
   auto preAlloc = (TCMallocStats::CurrentAllocatedBytes() >> B_TO_KB_SHIFT);
   auto preHeap = (TCMallocStats::HeapSizeInBytes() >> B_TO_KB_SHIFT);
  
   ForwardStats(statForwarder,{"TCMallocStatsTest"});

   const auto& results = statForwarder.mSendStatValues;
   const auto& keys = statForwarder.mSendStatKeys;
   const size_t TcMallocStatAmount = 4; // number of stats for TCMalloc
   ASSERT_EQ(results.size(), TcMallocStatAmount);
   ASSERT_EQ(keys.size(), TcMallocStatAmount);
   EXPECT_EQ(keys[0], "TCMallocStatsTest_Allocated");
   EXPECT_EQ(keys[1], "TCMallocStatsTest_Heap_Size");
   EXPECT_EQ(keys[2], "TCMallocStatsTest_Small_Object_Thread_Cache");
   EXPECT_EQ(keys[3], "TCMallocStatsTest_Memory_Slack");

   size_t smallArbitrary = 3; //KBytes
   EXPECT_GT(results[0], smallArbitrary);
   EXPECT_GT(results[1], smallArbitrary);
   EXPECT_GT(results[2], smallArbitrary);
   EXPECT_GT(results[3], smallArbitrary);

   // measurements pre should be smaller or equal (technically slack, cache could get smaller)
   EXPECT_TRUE(results[0] >= preAlloc);
   EXPECT_TRUE(results[1] >= preHeap);
   // cache is not measured since it MIGHT decrease
   // dito for slack
   
   // measurements post should be greater or equal (technically slack, cache could get smaller)
   EXPECT_TRUE(results[0] <= (TCMallocStats::CurrentAllocatedBytes() >> B_TO_KB_SHIFT));
   EXPECT_TRUE(results[1] <= (TCMallocStats::HeapSizeInBytes() >> B_TO_KB_SHIFT));
   // cache is not measured since it MIGHT decrease
   // dito for slack
   
}