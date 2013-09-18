#include "DiskCleanupTest.h"
#include "DiskCleanup.h"
#include "DiskPacketCapture.h"

#include "SendStats.h"
#include "MockConf.h"
#include "MockElasticSearch.h"
#include <future>
#include <thread>
#include <atomic>

#ifdef LR_DEBUG

TEST_F(DiskCleanupTest, ValgrindGetListToRemove) {
   MockElasticSearch es(false);
   es.mUpdateDocAlwaysPasses = true;

   MockDiskCleanup capture(mConf);
   size_t maxToRemove = 50000;
   size_t filesRemoved(0);
   size_t spaceRemoved(0);
   boost::filesystem::path path = "/usr/local/probe/pcap";
   for (int i = 0; i < 5 && ! zctx_interrupted; i ++) {

      std::map < std::time_t, std::vector<boost::filesystem::path> > fileOrderedByTime;
      for (int j = 10000000; j < 11000000; j ++) {
         std::vector<boost::filesystem::path > bucket;
         int bucketDepth = rand() % 10;
         for (int k = 100; k < 100 + bucketDepth; k ++) {
            std::string filename = std::to_string(j);
            filename += "1234567890123456789012345";
            filename += std::to_string(k);
            bucket.push_back(filename);
         }

         fileOrderedByTime[i * 10000000 + j] = bucket;
      }
      {
         std::vector< std::tuple< std::string, std::string> > filesToRemove =
                 capture.GetListToRemove(fileOrderedByTime, maxToRemove, filesRemoved, spaceRemoved);
         for (auto id : filesToRemove) {
            es.mQueryIdResults.push_back(std::make_pair(std::get<1>(id), "index_1973-11-29"));
         }
         capture.MarkFilesAsRemovedInES(filesToRemove, es);
      }
      std::cout << "iteration " << i << std::endl;
      es.mQueryIdResults.clear();
   }

}

TEST_F(DiskCleanupTest, ValgrindGetOrderedMapOfFiles) {
   MockElasticSearch es(false);
   es.mUpdateDocAlwaysPasses = true;

   MockDiskCleanup capture(mConf);
   size_t maxToRemove = 50000;
   size_t filesRemoved(0);
   size_t spaceRemoved(0);
   boost::filesystem::path path = "/usr/local/probe/pcap";
   for (int i = 0; i < 5 && ! zctx_interrupted; i ++) {

      std::map < std::time_t, std::vector<boost::filesystem::path> > fileOrderedByTime =
              capture.GetOrderedMapOfFiles(path);

      std::cout << "iteration " << i << std::endl;
   }

}
#endif

TEST_F(DiskCleanupTest, TooMuchPCap) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockElasticSearch es(false);
      es.mUpdateDocAlwaysPasses = true;
      es.RunQueryGetIdsAlwaysPasses = true;
      MockDiskCleanup capture(mConf);

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
      capture.ResetConf();
      std::atomic<size_t> aDiskUsed(0);
      std::atomic<size_t> aTotalFiles(0);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      mConf.mConf.mPCapCaptureLocation = testDir.str();

      std::string makeADir;

      ASSERT_EQ(0, system(makeADir.c_str()));
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup2";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup3";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup4";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup5";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M";

      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup6";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      make1MFileFile = "dd bs=1048575 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      size_t filesRemoved;
      size_t spaceSaved;
      capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved);
      EXPECT_EQ(1, filesRemoved);
      EXPECT_EQ(0, spaceSaved);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup8";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved);
      EXPECT_EQ(1, filesRemoved);
      EXPECT_EQ(1048576, spaceSaved);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved);
      EXPECT_EQ(1, filesRemoved);
      EXPECT_EQ(1048575, spaceSaved);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

   }
#endif
}

TEST_F(DiskCleanupTest, CleanupOldPcapFiles) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {



      MockDiskCleanup capture(mConf);
      SendStats sendQueue;
      size_t fsFreeGigs(0);
      size_t fsTotalGigs(100);
      MockElasticSearch es(false);
      DiskCleanup::PacketCaptureFilesystemDetails previous;
      previous.Writes = 0;
      previous.Reads = 0;
      previous.WriteBytes = 0;
      previous.ReadBytes = 0;
      std::time_t currentTime = std::time(NULL);
      std::atomic<size_t> aDiskUsed(0);
      std::atomic<size_t> aTotalFiles(0);

      es.mUpdateDocAlwaysPasses = true;
      es.RunQueryGetIdsAlwaysPasses = true;
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      std::string makeADir;

      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M";
      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      make1MFileFile = "dd bs=1048575 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup8";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

   }
#endif
}
#ifdef LR_DEBUG

TEST_F(DiskCleanupTest, ESFailuresGoAheadAndRemoveFiles) {
   if (geteuid() == 0) {
      std::string makeADir;
      SendStats sendQueue;
      size_t fsFreeGigs(0);
      size_t fsTotalGigs(100);
      MockElasticSearch es(false);
      DiskCleanup::PacketCaptureFilesystemDetails previous;
      previous.Writes = 0;
      previous.Reads = 0;
      previous.WriteBytes = 0;
      previous.ReadBytes = 0;
      std::time_t currentTime = std::time(NULL);
      std::atomic<size_t> aDiskUsed(0);
      std::atomic<size_t> aTotalFiles(0);
      MockDiskCleanup capture(mConf);
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/161122fd-6681-42a3-b953-48beb5247172";
      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.mFailUpdateDoc = true;
      es.mQueryIdResults.push_back(std::make_pair("161122fd-6681-42a3-b953-48beb5247172", "index_1973-11-29"));
      size_t filesRemoved;
      size_t spaceSaved;
      EXPECT_EQ(0, capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved));
      EXPECT_EQ(1, filesRemoved);
      EXPECT_EQ(0, spaceSaved);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.mFailUpdateDoc = true;
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

   }
}

TEST_F(DiskCleanupTest, ESFailsToQueryRemoveFile) {
   if (geteuid() == 0) {
      std::string makeADir;
      SendStats sendQueue;
      size_t fsFreeGigs(0);
      size_t fsTotalGigs(100);
      MockElasticSearch es(false);
      DiskCleanup::PacketCaptureFilesystemDetails previous;
      previous.Writes = 0;
      previous.Reads = 0;
      previous.WriteBytes = 0;
      previous.ReadBytes = 0;
      std::time_t currentTime = std::time(NULL);
      std::atomic<size_t> aDiskUsed(0);
      std::atomic<size_t> aTotalFiles(0);
      MockDiskCleanup capture(mConf);
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/161122fd-6681-42a3-b953-48beb5247172";
      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.RunQueryGetIdsAlwaysFails = true;
      es.mQueryIdResults.push_back(std::make_pair("161122fd-6681-42a3-b953-48beb5247172", "index_1973-11-29"));
      size_t filesRemoved;
      size_t spaceSaved;
      EXPECT_EQ(0, capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved));
      EXPECT_EQ(1, filesRemoved);
      EXPECT_EQ(0, spaceSaved);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
   }
}

TEST_F(DiskCleanupTest, TooMuchSearch) {

   MockDiskCleanup diskCleanup(mConf);

   EXPECT_FALSE(diskCleanup.TooMuchSearch(0, 0));
   EXPECT_TRUE(diskCleanup.TooMuchSearch(0, 100));
   EXPECT_TRUE(diskCleanup.TooMuchSearch(14, 100));
   EXPECT_FALSE(diskCleanup.TooMuchSearch(15, 100));
   EXPECT_FALSE(diskCleanup.TooMuchSearch(100, 100));
   EXPECT_FALSE(diskCleanup.TooMuchSearch(1000, 100));
}

TEST_F(DiskCleanupTest, RemoveOldestSearchFailureDoesntCrash) {


   MockDiskCleanup diskCleanup(mConf);
   diskCleanup.mFailRemoveSearch = true;
   diskCleanup.mFailFileSystemInfo = true;
   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool> finished, MockDiskCleanup & diskCleanup) {
      size_t fsFreeGigs(0);
      SendStats sendQueue;
              size_t fsTotalGigs(100);
              MockElasticSearch es(false);
              DiskCleanup::PacketCaptureFilesystemDetails previous;
              previous.Writes = 0;
              previous.Reads = 0;
              previous.WriteBytes = 0;
              previous.ReadBytes = 0;
              std::time_t currentTime = std::time(NULL);
              std::atomic<size_t> aDiskUsed(0);
              std::atomic<size_t> aTotalFiles(0);

              diskCleanup.CleanupSearch(false, previous, std::ref(es), sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
              finished.set_value(true);
   }, std::move(promisedFinished), std::ref(diskCleanup)).detach();

   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(100)) != std::future_status::timeout);

}

TEST_F(DiskCleanupTest, CleanupContinuouslyChecksSizes) {
   MockDiskCleanup diskCleanup(mConf);

   diskCleanup.mFileSystemInfoCountdown = 3;
   diskCleanup.mFailFileSystemInfo = true;
   diskCleanup.mSucceedRemoveSearch = true;

   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool> finished, MockDiskCleanup & diskCleanup) {
      size_t fsFreeGigs(0);
      SendStats sendQueue;
              size_t fsTotalGigs(100);
              MockElasticSearch es(false);
              DiskCleanup::PacketCaptureFilesystemDetails previous;
              previous.Writes = 0;
              previous.Reads = 0;
              previous.WriteBytes = 0;
              previous.ReadBytes = 0;
              std::time_t currentTime = std::time(NULL);
              std::atomic<size_t> aDiskUsed(0);
              std::atomic<size_t> aTotalFiles(0);
              diskCleanup.CleanupSearch(false, previous, std::ref(es), sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
      if (fsFreeGigs != fsTotalGigs) {
         FAIL();
      }
      finished.set_value(true);
   }, std::move(promisedFinished), std::ref(diskCleanup)).detach();

   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(100)) != std::future_status::timeout);
}

TEST_F(DiskCleanupTest, RemoveGetsTheOldestMatch) {
   MockDiskCleanup diskCleanup(mConf);
   MockElasticSearch es(false);

   es.mFakeIndexList = true;

   EXPECT_EQ("network_1999_01_01", diskCleanup.GetOldestIndex(es));

}

TEST_F(DiskCleanupTest, FSMath) {
   MockDiskCleanup diskCleanup(mConf);
   diskCleanup.mFleSystemInfo.f_bfree = 100 << B_TO_MB_SHIFT;
   diskCleanup.mFleSystemInfo.f_frsize = 1024;
   diskCleanup.mFleSystemInfo.f_blocks = 109 << B_TO_MB_SHIFT;
   diskCleanup.mFleSystemInfo.f_frsize = 1024;

   size_t free(0);
   size_t total(0);

   diskCleanup.GetFileSystemInfo(free, total);
   EXPECT_EQ(100, free);
   EXPECT_EQ(109, total);

   diskCleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
   diskCleanup.ResetConf();
   diskCleanup.GetFileSystemInfo(free, total);
   EXPECT_NE(0, free);
   EXPECT_NE(0, total);
}

TEST_F(DiskCleanupTest, DontDeleteTheLastIndex) {
   MockDiskCleanup diskCleanup(mConf);
   MockElasticSearch es(false);
   es.mMockListOfIndexes.clear();
   es.mFakeIndexList = true;
   std::string oldestIndex = diskCleanup.GetOldestIndex(es);
   EXPECT_EQ("", oldestIndex);
   es.mMockListOfIndexes.insert("network_12345");
   oldestIndex = diskCleanup.GetOldestIndex(es);
   EXPECT_EQ("", oldestIndex);
   es.mMockListOfIndexes.insert("network_12346");
   oldestIndex = diskCleanup.GetOldestIndex(es);
   EXPECT_EQ("network_12345", oldestIndex);
}
#endif
