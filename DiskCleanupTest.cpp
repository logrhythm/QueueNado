#include "DiskCleanupTest.h"
#include "MockConf.h"
#include <future>
#include <thread>
#include <atomic>

TEST_F(DiskCleanupTest, TooMuchPCap) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {

      MockDiskCleanup capture(mConf);

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
      std::atomic<size_t> aDiskUsed(0);
      std::atomic<size_t> aTotalFiles(0);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      std::stringstream testDir;
      testDir << "/tmp/TooMuchPcap";

      mConf.mConf.mPCapCaptureLocation = testDir.str();

      std::string makeADir;
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      system(makeADir.c_str());

      makeADir = "mkdir -p ";
      makeADir += testDir.str();
      system(makeADir.c_str());

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
      makeSmallFile += "/smallFile";

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup5";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";

      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup6";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      make1MFileFile = "dd bs=1048576 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFilelessone";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      capture.RemoveOldestPCapFiles(1);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup8";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.RemoveOldestPCapFiles(1);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.RemoveOldestPCapFiles(1);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
   }
#endif
}

TEST_F(DiskCleanupTest, CleanupOldPcapFiles) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {

      MockDiskCleanup capture(mConf);
      std::atomic<size_t> aDiskUsed(0);
      std::atomic<size_t> aTotalFiles(0);
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      std::stringstream testDir;
      testDir << "/tmp/TooMuchPcap";

      std::string makeADir;
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      system(makeADir.c_str());

      makeADir = "mkdir -p ";
      makeADir += testDir.str();
      system(makeADir.c_str());

      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/smallFile";
      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      make1MFileFile = "dd bs=1048576 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFilelessone";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup8";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
   }
#endif
}
#ifdef LR_DEBUG

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
   std::thread([](std::promise<bool>& finished, MockDiskCleanup & diskCleanup) {
      size_t free(0);
      size_t total(100);
              diskCleanup.CleanupSearch(free, total);
              finished.set_value(true);
   }, std::ref(promisedFinished), std::ref(diskCleanup)).detach();

   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(100)) != std::future_status::timeout);

}

TEST_F(DiskCleanupTest, CleanupContinuouslyChecksSizes) {
   MockDiskCleanup diskCleanup(mConf);
   diskCleanup.mFileSystemInfoCountdown = 3;
   diskCleanup.mFailFileSystemInfo = true;
   diskCleanup.mSucceedRemoveSearch = true;

   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool>& finished, MockDiskCleanup & diskCleanup) {
      size_t free(0);
      size_t total(100);
              diskCleanup.CleanupSearch(free, total);
      if (free != total) {
         FAIL();
      }
      finished.set_value(true);
   }, std::ref(promisedFinished), std::ref(diskCleanup)).detach();

   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(100)) != std::future_status::timeout);
}

TEST_F(DiskCleanupTest, FSMath) {
   MockDiskCleanup diskCleanup(mConf);
   diskCleanup.mFleSystemInfo.f_bfree = 100 << B_TO_MB_SHIFT;
   diskCleanup.mFleSystemInfo.f_frsize = 1024;
   diskCleanup.mFleSystemInfo.f_blocks = 109 << B_TO_MB_SHIFT;
   diskCleanup.mFleSystemInfo.f_frsize = 1024;
   
   size_t free(0);
   size_t total(0);
   
   diskCleanup.GetFileSystemInfo(free,total);
   EXPECT_EQ(100,free);
   EXPECT_EQ(109,total);
}
#endif