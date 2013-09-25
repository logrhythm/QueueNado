#include <gtest/gtest.h>
#include <g2log.hpp>
#include <string>

#include "TCMallocStats.h"
#include <iostream> // for disabled test


// Disabled since for unit test the TCMALLOC_....REPORT_THRESHOLD is usually not set
// You can run this test by doing:
//
//  sudo TCMALLOC_LARGE_ALLOC_REPORT_THRESHOLD=16106127360 ./ProcStatsTest  --gtest_also_run_disabled_tests --gtest_filter="*TCMalloc*" 
//
TEST(TCMallocStatsTest, DISABLED_GetEnvReportThreshold) {
  using namespace TCMallocStats;
  auto threshold = AllocReportThreshold();
  EXPECT_GT(threshold, 0); 
  LOG(INFO) << "AllocReportThreshold: " << threshold;
  std::cerr << "AllocReportThreshold: " << threshold << std::endl;
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
   
   EXPECT_GT(releaseRate, 0);
   //EXPECT_GT(allocReport, 0); 
   EXPECT_GT(maxTotalThreadCacheBytes, 0);
   
   EXPECT_GT(currentBytes, 0);
   EXPECT_GT(heapSize, 0);
   EXPECT_GT(pageHeapFreeBytes, 0);
   EXPECT_GT(pageHeapUnmappedBytes, 0);
   EXPECT_GT(slackBytes, 0);
   EXPECT_GT(currentTotalThreadCacheBytes, 0);
   
   SendToLogAllStats(); // just for visual comparison
}