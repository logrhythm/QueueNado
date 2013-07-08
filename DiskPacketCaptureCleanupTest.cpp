#include "DiskPacketCaptureCleanupTest.h"
#include "MockConf.h"

TEST_F(DiskPacketCaptureCleanupTest, TooMuchPCap) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockConf conf;
      MockDiskPacketCaptureCleanup capture(conf);

      conf.mPCapCaptureLocation = "testLocation";
      conf.mPCapCaptureFileLimit = 10000;
      conf.mPCapCaptureSizeLimit = 10000;
      std::atomic<size_t> aDiskUsed(0);
      std::atomic<size_t> aTotalFiles(0);
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));

      std::stringstream testDir;
      testDir << "/tmp/TooMuchPcap." << pthread_self() << getpid();

      conf.mPCapCaptureLocation = testDir.str();

      std::string makeADir = "mkdir -p ";
      makeADir += testDir.str();

      ASSERT_EQ(0, system(makeADir.c_str()));
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureFileLimit = 0;
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureFileLimit = 10000;
      conf.mPCapCaptureSizeLimit = 0;
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureFileLimit = 1;
      conf.mPCapCaptureSizeLimit = 1;
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/smallFile";

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureFileLimit = 10;
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";


      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureSizeLimit = 3;
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      make1MFileFile = "dd bs=1048576 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFilelessone";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));

      conf.mPCapCaptureFileLimit = 3;
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      
      capture.RemoveOldestPCapFile();
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureSizeLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      capture.RemoveOldestPCapFile();
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureFileLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      capture.RemoveOldestPCapFile();
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));

      makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
   }
#endif
}

TEST_F(DiskPacketCaptureCleanupTest, CleanupOldPcapFiles) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockConf conf;
      MockDiskPacketCaptureCleanup capture(conf);
      std::atomic<size_t> aDiskUsed;
      std::atomic<size_t> aTotalFiles;
      conf.mPCapCaptureLocation = "testLocation";
      conf.mPCapCaptureFileLimit = 10000;
      conf.mPCapCaptureSizeLimit = 10000;
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));

      std::stringstream testDir;
      testDir << "/tmp/TooMuchPcap." << pthread_self() << getpid();

      conf.mPCapCaptureLocation = testDir.str();

      std::string makeADir = "mkdir -p ";
      makeADir += testDir.str();

      ASSERT_EQ(0, system(makeADir.c_str()));

      capture.Initialize();
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/smallFile";
      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      make1MFileFile = "dd bs=1048576 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFilelessone";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculateDiskUsed(aDiskUsed,aTotalFiles);
      conf.mPCapCaptureFileLimit = 3;
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      capture.CleanupOldPcapFiles(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureSizeLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      capture.CleanupOldPcapFiles(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      conf.mPCapCaptureFileLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));
      capture.CleanupOldPcapFiles(aDiskUsed,aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed,aTotalFiles));

      makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
   }
#endif
}