#include "DiskCleanupTest.h"
#include "DiskCleanup.h"
#include "DiskPacketCapture.h"
#include "tempFileCreate.h"
#include "SendStats.h"
#include "MockConf.h"
#include "MockElasticSearch.h"
#include <future>
#include <thread>
#include <atomic>

#ifdef LR_DEBUG

TEST_F(DiskCleanupTest, OptimizeThreadObeysShutdown) {
   GMockDiskCleanup cleanup(mConf);
   MockBoomStick transport("ipc://tmp/foo.ipc");
   MockElasticSearch es(transport, false);
   
   cleanup.DelegateIsShutdownAlwaysTrue();
   cleanup.OptimizeThread(es);
   EXPECT_EQ(1,cleanup.returnBools.callsToReturnTrue);
}
TEST_F(DiskCleanupTest, OptimizeThreadFailsWhenESBorked) {
   GMockDiskCleanup cleanup(mConf);
   MockBoomStick transport("ipc://tmp/foo.ipc");
   GMockElasticSearch es(transport, false);
   
   cleanup.DelegateIsShutdownAlwaysTrue();
   es.DelegateInitializeToAlwaysFail();
   cleanup.OptimizeThread(es);
   EXPECT_CALL(cleanup, IsShutdown())
      .Times(0);
   EXPECT_EQ(0,cleanup.returnBools.callsToReturnTrue);
}
TEST_F(DiskCleanupTest, OptimizeIndexes) {
   MockDiskCleanup cleanup(mConf);
   MockBoomStick transport("ipc://tmp/foo.ipc");
   MockElasticSearch es(transport, false);

   ASSERT_TRUE(es.Initialize());
   transport.mReturnString = "200|ok|{}";
   
   std::set<std::string> allIndexes;
   std::set<std::string> excludes;
   
   cleanup.OptimizeIndexes(allIndexes,excludes,es);
   EXPECT_TRUE(es.mOptimizedIndexes.empty());
   allIndexes.insert("test1");
   cleanup.OptimizeIndexes(allIndexes,excludes,es);
   EXPECT_TRUE(es.mOptimizedIndexes.find("test1")!=es.mOptimizedIndexes.end());
   es.mOptimizedIndexes.clear();
   allIndexes.insert("test2");
   excludes.insert("test2");
   cleanup.OptimizeIndexes(allIndexes,excludes,es);
   EXPECT_TRUE(es.mOptimizedIndexes.find("test1")!=es.mOptimizedIndexes.end());
   EXPECT_FALSE(es.mOptimizedIndexes.find("test2")!=es.mOptimizedIndexes.end());
}

TEST_F(DiskCleanupTest, GetIndexesThatAreActive) {
   MockDiskCleanup cleanup(mConf);
   std::time_t now(std::time(NULL));
   
   networkMonitor::DpiMsgLR todayMsg;
   todayMsg.set_timeupdated(now);
   
   std::set<std::string> excludes = cleanup.GetIndexesThatAreActive();

   EXPECT_TRUE(excludes.find("kibana-int") != excludes.end());
   EXPECT_TRUE(excludes.find(todayMsg.GetESIndexName()) != excludes.end());
   todayMsg.set_timeupdated(now-(24*60*60));
   EXPECT_TRUE(excludes.find(todayMsg.GetESIndexName()) != excludes.end());
   todayMsg.set_timeupdated(now-(48*60*60));
   EXPECT_FALSE(excludes.find(todayMsg.GetESIndexName()) != excludes.end());
}
TEST_F(DiskCleanupTest, MarkFileAsRemovedInES) {
   MockDiskCleanup cleanup(mConf);
   MockBoomStick transport("ipc://tmp/foo.ipc");
   MockElasticSearch es(transport, false);
   IdsAndIndexes recordsToUpdate;

   EXPECT_FALSE(cleanup.MarkFilesAsRemovedInES(recordsToUpdate, es));
   recordsToUpdate.emplace_back("123456789012345678901234567890123456", "foo");
   es.mFakeBulkUpdate = true;
   es.mBulkUpdateResult = false;
   EXPECT_FALSE(cleanup.MarkFilesAsRemovedInES(recordsToUpdate, es));
   es.mBulkUpdateResult = true;
   EXPECT_TRUE(cleanup.MarkFilesAsRemovedInES(recordsToUpdate, es));
}

TEST_F(DiskCleanupTest, RemoveFile) {
   MockDiskCleanup cleanup(mConf);

   std::string path;
   path += testDir.str();
   path += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"; // 1 empty file

   mConf.mConf.mPCapCaptureLocations.push_back(testDir.str());
   std::string makeADir;
   ASSERT_EQ(0, system(makeADir.c_str()));
   std::string makeSmallFile = "touch ";
   makeSmallFile += path;

   EXPECT_EQ(0, system(makeSmallFile.c_str()));
   struct stat filestat;
   EXPECT_TRUE(stat(path.c_str(), &filestat) == 0);
   EXPECT_TRUE(cleanup.RemoveFile(path));
   EXPECT_FALSE(stat(path.c_str(), &filestat) == 0);
   EXPECT_TRUE(cleanup.RemoveFile(path)); // Missing is ok
   EXPECT_FALSE(stat(path.c_str(), &filestat) == 0);
}

TEST_F(DiskCleanupTest, RemoveFiles) {
   MockDiskCleanup cleanup(mConf);
   PathAndFileNames filesToRemove;
   size_t spaceSavedInMB(99999);
   struct stat filestat;
   std::string path;
   path += testDir.str();
   path += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"; // 1 empty file

   mConf.mConf.mPCapCaptureLocations.push_back(testDir.str());
   std::string makeADir;
   ASSERT_EQ(0, system(makeADir.c_str()));
   std::string makeSmallFile = "touch ";
   makeSmallFile += path;

   EXPECT_EQ(0, system(makeSmallFile.c_str()));
   size_t filesNotFound;
   EXPECT_EQ(0, cleanup.RemoveFiles(filesToRemove, spaceSavedInMB,filesNotFound));
   EXPECT_EQ(0, spaceSavedInMB);
   spaceSavedInMB = 999999;
   filesToRemove.emplace_back(path, "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
   EXPECT_TRUE(stat(path.c_str(), &filestat) == 0);
   EXPECT_EQ(0, cleanup.RemoveFiles(filesToRemove, spaceSavedInMB,filesNotFound));
   EXPECT_EQ(0, spaceSavedInMB);
   EXPECT_FALSE(stat(path.c_str(), &filestat) == 0);
   spaceSavedInMB = 999999;
   cleanup.mFakeRemove = true;
   cleanup.mRemoveResult = false;
   EXPECT_EQ(0, system(makeSmallFile.c_str()));
   EXPECT_EQ(1, cleanup.RemoveFiles(filesToRemove, spaceSavedInMB,filesNotFound));
   EXPECT_EQ(0, spaceSavedInMB);

   cleanup.mFakeRemove = false;

   std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
   path = testDir.str();
   path += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M";
   make1MFileFile += path;

   EXPECT_EQ(0, system(make1MFileFile.c_str()));

   filesToRemove.clear();
   filesToRemove.emplace_back(path, "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M");
   EXPECT_EQ(0, cleanup.RemoveFiles(filesToRemove, spaceSavedInMB,filesNotFound));
   EXPECT_EQ(1, spaceSavedInMB);

   EXPECT_FALSE(stat(path.c_str(), &filestat) == 0);

   EXPECT_EQ(0, system(make1MFileFile.c_str()));
   cleanup.mFakeIsShutdown = true;
   cleanup.mIsShutdownResult = true;
   EXPECT_EQ(0, cleanup.RemoveFiles(filesToRemove, spaceSavedInMB,filesNotFound));
   EXPECT_EQ(0, spaceSavedInMB);
   EXPECT_TRUE(stat(path.c_str(), &filestat) == 0);
}

TEST_F(DiskCleanupTest, GetOlderFilesFromPath) {
   MockDiskCleanup cleanup(mConf);   
   PathAndFileNames filesToFind;
   std::string path;
   path += testDir.str();
   path += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
   std::string makeSmallFile = "touch ";
   makeSmallFile += path;

   filesToFind = cleanup.GetOlderFilesFromPath(testDir.str(), std::time(NULL));
   EXPECT_TRUE(filesToFind.empty());

   EXPECT_EQ(0, system(makeSmallFile.c_str()));
   std::this_thread::sleep_for(std::chrono::seconds(1)); // increase timestamp
   cleanup.mFakeIsShutdown = true;
   cleanup.mIsShutdownResult = true;
   filesToFind = cleanup.GetOlderFilesFromPath(testDir.str(), std::time(NULL));
   EXPECT_TRUE(filesToFind.empty());

   cleanup.mFakeIsShutdown = false;
   filesToFind = cleanup.GetOlderFilesFromPath("/thisPathIsGarbage", 0);
   EXPECT_TRUE(filesToFind.empty());
   std::this_thread::sleep_for(std::chrono::seconds(1));
   filesToFind = cleanup.GetOlderFilesFromPath(testDir.str(), std::time(NULL));

   ASSERT_FALSE(filesToFind.empty());
   EXPECT_TRUE(std::get<0>(filesToFind[0]) == path);
   EXPECT_TRUE(std::get<1>(filesToFind[0]) == "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");

}

TEST_F(DiskCleanupTest, TimeToForceAClean) {
   MockDiskCleanup cleanup(mConf);

   cleanup.SetLastForcedClean(std::time(NULL));
   EXPECT_FALSE(cleanup.TimeForBruteForceCleanup());
   cleanup.SetLastForcedClean(std::time(NULL) - 20 * 60 - 1);
   EXPECT_TRUE(cleanup.TimeForBruteForceCleanup());
}

TEST_F(DiskCleanupTest, WayTooManyFiles) {
   MockDiskCleanup cleanup(mConf);


   mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
   cleanup.ResetConf();
   EXPECT_FALSE(cleanup.WayTooManyFiles(0));
   EXPECT_FALSE(cleanup.WayTooManyFiles(59999));
   EXPECT_FALSE(cleanup.WayTooManyFiles(60000));

   mConf.mConfLocation = "resources/test.yaml.DiskCleanup0"; // file limit 0
   cleanup.ResetConf();

   EXPECT_FALSE(cleanup.WayTooManyFiles(0)); // Don't add more to 100% of the files when we have a target of 0
}

TEST_F(DiskCleanupTest, CalculateNewTotalFiles) {

   MockDiskCleanup cleanup(mConf);
   EXPECT_EQ(0, cleanup.CalculateNewTotalFiles(0, 0, 0));
   EXPECT_EQ(0, cleanup.CalculateNewTotalFiles(0, 0, 1));
   EXPECT_EQ(0, cleanup.CalculateNewTotalFiles(0, 1, 0));
   EXPECT_EQ(0, cleanup.CalculateNewTotalFiles(0, 1, 1));
   EXPECT_EQ(1, cleanup.CalculateNewTotalFiles(1, 0, 0));
   EXPECT_EQ(1, cleanup.CalculateNewTotalFiles(1, 0, 1));
   EXPECT_EQ(0, cleanup.CalculateNewTotalFiles(1, 1, 0));
   EXPECT_EQ(1, cleanup.CalculateNewTotalFiles(1, 1, 1));
}

TEST_F(DiskCleanupTest, IterationTargetToRemove) {
   MockDiskCleanup cleanup(mConf);
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup11"; // file limit 30000000
   cleanup.ResetConf();
   EXPECT_EQ(0, cleanup.IterationTargetToRemove(0));
   EXPECT_EQ(1000, cleanup.IterationTargetToRemove(1));
   EXPECT_EQ(1000, cleanup.IterationTargetToRemove(49999));
   EXPECT_EQ(1001, cleanup.IterationTargetToRemove(50001));
   EXPECT_EQ(100000, cleanup.IterationTargetToRemove(5000100));
   EXPECT_EQ(100000, cleanup.IterationTargetToRemove(1000000000 ));

   mConf.mConfLocation = "resources/test.yaml.DiskCleanup0"; // file limit 0
   cleanup.ResetConf();
   EXPECT_EQ(0, cleanup.IterationTargetToRemove(0));
   EXPECT_EQ(1, cleanup.IterationTargetToRemove(1));
   EXPECT_EQ(50100, cleanup.IterationTargetToRemove(50100));

}

TEST_F(DiskCleanupTest, LastIterationAmount) {
   MockDiskCleanup cleanup(mConf);

   EXPECT_TRUE(cleanup.LastIterationAmount(0));
   EXPECT_TRUE(cleanup.LastIterationAmount(1));
   EXPECT_TRUE(cleanup.LastIterationAmount(1000));
   EXPECT_FALSE(cleanup.LastIterationAmount(1001));

   mConf.mConfLocation = "resources/test.yaml.DiskCleanup2"; // file limit 1
   cleanup.ResetConf();
   EXPECT_TRUE(cleanup.LastIterationAmount(0));
   EXPECT_TRUE(cleanup.LastIterationAmount(1));
   EXPECT_FALSE(cleanup.LastIterationAmount(2));
   EXPECT_FALSE(cleanup.LastIterationAmount(1001));

   mConf.mConfLocation = "resources/test.yaml.DiskCleanup0"; // file limit 0
   cleanup.ResetConf();
   EXPECT_TRUE(cleanup.LastIterationAmount(0));
   EXPECT_TRUE(cleanup.LastIterationAmount(1));
   EXPECT_TRUE(cleanup.LastIterationAmount(2));
   EXPECT_TRUE(cleanup.LastIterationAmount(1001));
}

TEST_F(DiskCleanupTest, CleanupMassiveOvershoot) {
   MockDiskCleanup cleanup(mConf);
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup0"; // file limit 0
   cleanup.ResetConf();
   EXPECT_EQ(100, cleanup.CleanupMassiveOvershoot(0, 1000));
   EXPECT_EQ(110, cleanup.CleanupMassiveOvershoot(10, 1000));
   EXPECT_EQ(1000, cleanup.CleanupMassiveOvershoot(901, 1000));
   EXPECT_EQ(1000, cleanup.CleanupMassiveOvershoot(10000, 1000));
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup1"; // file limit 30000
   cleanup.ResetConf();
   EXPECT_EQ(100, cleanup.CleanupMassiveOvershoot(0, 30000 + 1000));
   EXPECT_EQ(110, cleanup.CleanupMassiveOvershoot(10, 30000 + 1000));
   EXPECT_EQ(1000, cleanup.CleanupMassiveOvershoot(901, 30000 + 1000));
   EXPECT_EQ(1000, cleanup.CleanupMassiveOvershoot(10000, 30000 + 1000));
}

TEST_F(DiskCleanupTest, DISABLED_ValgrindGetOrderedMapOfFiles) {
   MockElasticSearch es(false);
   es.mUpdateDocAlwaysPasses = true;

   MockDiskCleanup capture(mConf);
   size_t maxToRemove = 5000;
   size_t filesRemoved(0);
   size_t spaceRemoved(0);
   boost::filesystem::path path = "/usr/local/probe/pcap";
   for (int i = 0; i < 1 && !zctx_interrupted; i++) {

      std::vector< std::tuple< std::string, std::string> >& oldestFiles =
              capture.GetOlderFilesFromPath(path, std::time(NULL));

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
      capture.mRealFilesSystemAccess = true;
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
      capture.ResetConf();
      size_t aDiskUsed(0);
      size_t aTotalFiles(0);
      DiskCleanup::DiskSpace ignored;
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));


      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup2";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup3";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup4";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"; // 1 empty file

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      time_t timeFirst = std::time(NULL);
      std::this_thread::sleep_for(std::chrono::seconds(1));

      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_EQ(aDiskUsed, 0); // 0MB
      EXPECT_EQ(aTotalFiles, 1);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles)); // 1 file, limit 1 file
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::KByte), 4);
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::MB), 0);

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup5";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M";

      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      time_t timeSecond = std::time(NULL);
      std::this_thread::sleep_for(std::chrono::seconds(1));

      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::MB), 1); // 2 files: 1MB + (4KByte folder overhead)

      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup6";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      // 1 byte less than 1 MB. Keep subtracting that from the calculations below
      size_t byte = 1;

      make1MFileFile = "dd bs=1048575 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      time_t timeThird = std::time(NULL);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      // 3 files: 2MB + (4KByte folder overhead) - 1byte
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte),
              (2 << B_TO_MB_SHIFT) + (4 << B_TO_KB_SHIFT) - byte);


      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      size_t spaceSaved(0);
      auto location = capture.GetConf().GetFirstPcapCaptureLocation();
      EXPECT_EQ(testDir.str() + "/", location);
      EXPECT_EQ(1, capture.BruteForceCleanupOfOldFiles(location, timeSecond, spaceSaved)); // 2 files, empty file removed
      EXPECT_EQ(0, spaceSaved);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(2, aTotalFiles);
      // 2MB + (4KByte folder overhead) - 1byte
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte),
              (2 << B_TO_MB_SHIFT) + (4 << B_TO_KB_SHIFT) - byte);



      mConf.mConfLocation = "resources/test.yaml.DiskCleanup8";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(aTotalFiles, 2);
      EXPECT_EQ(1, capture.BruteForceCleanupOfOldFiles(testDir.str(), timeThird, spaceSaved));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_EQ(1, aTotalFiles);
      EXPECT_EQ(1, spaceSaved); //left is 1 file: 1MB-1byte + (4KByte folder overhead) 
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte),
              (1 << B_TO_MB_SHIFT) + (4 << B_TO_KB_SHIFT) - byte);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles)); // 1MB limit vs 1MB-1byte + 4KByte

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9"; // 1MB limit, 1 file limit
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(1, capture.BruteForceCleanupOfOldFiles(testDir.str(), std::time(NULL), spaceSaved));
      EXPECT_EQ(1, spaceSaved); // 
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte),
              (0 << B_TO_MB_SHIFT) + (4 << B_TO_KB_SHIFT));

      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_EQ(0, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
   }
#endif
}


TEST_F(DiskCleanupTest, TooMuchPCapsAtManyLocations) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockElasticSearch es(false);
      es.mUpdateDocAlwaysPasses = true;
      es.RunQueryGetIdsAlwaysPasses = true;

      MockDiskCleanup capture(mConf);
      capture.mRealFilesSystemAccess = true;
      mConf.mConfLocation = "resources/test.yaml.GetPcapStoreUsage1";
      ASSERT_EQ(0, system("mkdir -p /tmp/TooMuchPcap/pcap0"));
      ASSERT_EQ(0, system("mkdir -p /tmp/TooMuchPcap/pcap1"));
      capture.ResetConf();
      Conf& conf = capture.GetConf();
      auto locations = conf.GetPcapCaptureLocations();
      ASSERT_EQ(locations.size(), 2) << conf.GetFirstPcapCaptureLocation();
      ASSERT_EQ(locations[0], "/tmp/TooMuchPcap/pcap0/");
      ASSERT_EQ(locations[1], "/tmp/TooMuchPcap/pcap1/");
      
      size_t aDiskUsed(0);
      size_t aTotalFiles(0);
      DiskCleanup::DiskSpace ignored;
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(aTotalFiles, 0);
      
      std::string makeFile1 = "dd bs=1024 count=1024 if=/dev/zero of=";
      makeFile1 += locations[0];
      makeFile1 += "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1";
      std::string makeFile2 = "dd bs=1024 count=1024 if=/dev/zero of=";
      makeFile2 += locations[1];
      makeFile2 += "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1";
      EXPECT_EQ(system(makeFile1.c_str()), 0) << makeFile1;
      EXPECT_EQ(system(makeFile2.c_str()), 0) << makeFile2;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      time_t timeFirst = std::time(NULL);


      EXPECT_TRUE(boost::filesystem::exists(locations[0] + "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1"));
      EXPECT_TRUE(boost::filesystem::exists(locations[1] + "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1"));
      
      // Clean it up
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_EQ(aTotalFiles, 2);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      size_t spaceSaved = 0;
     
      // Remove the first file
      EXPECT_EQ(1, capture.BruteForceCleanupOfOldFiles(locations[0], timeFirst, spaceSaved));
      EXPECT_EQ(1, spaceSaved); 
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_EQ(1, aTotalFiles);
      EXPECT_FALSE(boost::filesystem::exists(locations[0] + "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1"));
      EXPECT_TRUE(boost::filesystem::exists(locations[1] + "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1"));
      // Remove the second file
      EXPECT_EQ(1, capture.BruteForceCleanupOfOldFiles(locations[1], timeFirst, spaceSaved));
      EXPECT_EQ(1, spaceSaved); 
      EXPECT_FALSE(boost::filesystem::exists(locations[0] + "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1"));
      EXPECT_FALSE(boost::filesystem::exists(locations[1] + "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1"));
      
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, ignored);
      EXPECT_EQ(0, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
   }
#endif
}

#ifdef LR_DEBUG

TEST_F(DiskCleanupTest, SystemTest_GetPcapStoreUsageSamePartition) {
   MockDiskCleanup cleanup(mConf);
   cleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
   cleanup.ResetConf();
   Conf& conf = cleanup.GetConf();
   EXPECT_EQ(conf.GetFirstPcapCaptureLocation(), "/tmp/TooMuchPcap/");
   EXPECT_EQ(conf.GetPcapCaptureLocations()[0], "/tmp/TooMuchPcap/");
   
   EXPECT_EQ(conf.GetProbeLocation(), "/tmp/TooMuchPcap");

   // Same partition: disk usage for pcap is calulcated on the folder itself
   // free and total should be huge since it is on the root partition
   DiskCleanup::DiskSpace pcapDisk{0, 0, 0};
   cleanup.GetPcapStoreUsage(pcapDisk, DiskUsage::Size::KByte);
   size_t spaceToCreateADirectory = 4;
   EXPECT_EQ(pcapDisk.Used, spaceToCreateADirectory); // overhead creating dir
           
   std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
   make1MFileFile += testDir.str();
   make1MFileFile += "/1MFile";
   EXPECT_EQ(0, system(make1MFileFile.c_str()));
   size_t usedKByte = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::KByte);
   EXPECT_EQ(usedKByte, 1024 + spaceToCreateADirectory); // including 4: overhead

   cleanup.GetPcapStoreUsage(pcapDisk, DiskUsage::Size::KByte);
   EXPECT_EQ(pcapDisk.Used, 1024 + spaceToCreateADirectory);
}
#endif

#ifdef LR_DEBUG
// Could also be DISABLED since it used two partitions for verifying that the actual
// partition calculation is OK. On our test system (Jenkins?) all is on the
// same partition so in this case we do not run this test
//
// There is no point in mocking this and using folders on the same partition
// since the test is for verifying how it works on partitions
TEST_F(DiskCleanupTest, SystemTest_GetPcapStoreUsageManyLocations) {
   DiskUsage home("/home/tmp/TooMuchPcap");
   DiskUsage root("/tmp/TooMuchPcap");
   if (home.FileSystemID() != root.FileSystemID()) {
      MockDiskCleanup cleanup(mConf);
      cleanup.mRealFilesSystemAccess = true;
      cleanup.mUseMockConf = true;
      auto& mockedConf = cleanup.mMockedConf;
      tempFileCreate scopedHome(cleanup.mMockedConf, "/home/tmp/TooMuchPcap");
      tempFileCreate scopedRoot(cleanup.mMockedConf, "/tmp/TooMuchPcap");

      ASSERT_TRUE(scopedHome.Init());
      ASSERT_TRUE(scopedRoot.Init());

      mockedConf.mPCapCaptureLocations.clear();
      mockedConf.mPCapCaptureLocations.push_back(scopedHome.mTestDir.str());
      mockedConf.mPCapCaptureLocations.push_back(scopedRoot.mTestDir.str());



      ASSERT_EQ(mockedConf.GetPcapCaptureLocations().size(), 2);
      ASSERT_EQ(mockedConf.GetPcapCaptureLocations()[0], scopedHome.mTestDir.str());
      ASSERT_EQ(mockedConf.GetPcapCaptureLocations()[1], scopedRoot.mTestDir.str());
      EXPECT_EQ(mockedConf.GetFirstPcapCaptureLocation(), scopedHome.mTestDir.str());
      Conf& conf = cleanup.GetConf();
      EXPECT_EQ(conf.GetFirstPcapCaptureLocation(), scopedHome.mTestDir.str());
      EXPECT_EQ(conf.GetPcapCaptureLocations()[0], scopedHome.mTestDir.str());
      EXPECT_EQ(conf.GetPcapCaptureLocations()[1], scopedRoot.mTestDir.str());
      EXPECT_EQ(mockedConf.GetProbeLocation(), "/usr/local/probe/");


      // Using 2 different partitions for pcaps
      DiskCleanup::DiskSpace pcapDisk{0, 0, 0};
      auto size = DiskUsage::Size::MB;
      cleanup.GetPcapStoreUsage(pcapDisk, size); // high granularity in case something changes on the system
      DiskUsage atRoot{scopedRoot.mTestDir.str()};
      DiskUsage atHome(scopedHome.mTestDir.str());

      auto isFree = atRoot.DiskFree(size) + atHome.DiskFree(size);
      auto isUsed = atRoot.DiskUsed(size) + atHome.DiskUsed(size);
      auto isTotal = atRoot.DiskTotal(size) + atHome.DiskTotal(size);
      EXPECT_EQ(pcapDisk.Free, isFree) << ". home: " << atHome.DiskFree(size) << ". root:" << atRoot.DiskFree(size);
      EXPECT_EQ(pcapDisk.Used, isUsed) << ". home: " << atHome.DiskUsed(size) << ". root:" << atRoot.DiskUsed(size);
      EXPECT_EQ(pcapDisk.Total, isTotal) << ". home: " << atHome.DiskTotal(size) << ". root:" << atRoot.DiskTotal(size);


      // Sanity check. Add a 10MFile and see that increments happen
      std::string make1MFileFile = "dd bs=1024 count=10240 if=/dev/zero of=";
      make1MFileFile += scopedHome.mTestDir.str();
      make1MFileFile += "/10MFile";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      DiskUsage atHome2(scopedHome.mTestDir.str());
      size_t usedMByte = atHome2.DiskUsed(size);
      cleanup.GetPcapStoreUsage(pcapDisk, size);
      EXPECT_EQ(pcapDisk.Used, isUsed + 10);
      EXPECT_EQ(pcapDisk.Used, usedMByte + atRoot.DiskUsed(size));
   }
}
#endif





#ifdef LR_DEBUG
TEST_F(DiskCleanupTest, GetProbeDiskUsage) {
   MockDiskCleanup cleanup(mConf);
   cleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
   cleanup.ResetConf();
   Conf& conf = cleanup.GetConf();

   
   EXPECT_EQ(conf.GetPcapCaptureLocations()[0], "/tmp/TooMuchPcap/");
   EXPECT_EQ(conf.GetProbeLocation(), "/tmp/TooMuchPcap");

   // Same partition: disk usage for pcap is calulcated on the folder itself
   // free and total should be huge since it is on the root partition
   DiskCleanup::DiskSpace pcapDiskInKByte{0, 0, 0};
   DiskCleanup::DiskSpace probeDiskInKByte{0, 0, 0};

   cleanup.GetPcapStoreUsage(pcapDiskInKByte, DiskUsage::Size::KByte);
   cleanup.GetProbeFileSystemInfo(probeDiskInKByte, DiskUsage::Size::KByte);

   // Same partition. For "free" and "total" used memory should be the same for 
   //  GetTotal..., GetProbe... and getPcap
   EXPECT_EQ(pcapDiskInKByte.Free, probeDiskInKByte.Free);
   EXPECT_EQ(pcapDiskInKByte.Total, probeDiskInKByte.Total);
   EXPECT_NE(pcapDiskInKByte.Used, probeDiskInKByte.Used); // pcap is the folder, probe is the partition
   EXPECT_EQ(pcapDiskInKByte.Used, 4); // folder takes up space
   EXPECT_NE(probeDiskInKByte.Used, 4); // probe is the whole partition
   EXPECT_TRUE(probeDiskInKByte.Used > 4);

}
#endif

#ifdef LR_DEBUG
TEST_F(DiskCleanupTest, SystemTest_RecalculatePCapDiskUsedSamePartition) {
   MockDiskCleanup cleanup(mConf);
   cleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
   cleanup.ResetConf();
   Conf& conf = cleanup.GetConf();

   EXPECT_EQ(conf.GetFirstPcapCaptureLocation(), "/tmp/TooMuchPcap/");
   EXPECT_EQ(conf.GetPcapCaptureLocations()[0], "/tmp/TooMuchPcap/");
   EXPECT_EQ(conf.GetProbeLocation(), "/tmp/TooMuchPcap");

   // Same partition: disk usage for pcap is calulcated on the folder itself
   // free and total should be huge since it is on the root partition
   DiskCleanup::DiskSpace pcapDisk{0, 0, 0};
   size_t diskUsed_1_MB;
   size_t totalFiles;
   cleanup.RecalculatePCapDiskUsed(diskUsed_1_MB, totalFiles, pcapDisk);   
   EXPECT_EQ(totalFiles, 0);
   size_t usedMB = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::MB);
   EXPECT_EQ(usedMB, 0);
   EXPECT_EQ(usedMB, diskUsed_1_MB);
   
   std::string make10MFile = "dd bs=1024 count=10240 if=/dev/zero of=";
   make10MFile += testDir.str();
   make10MFile += "/10MFile";
   EXPECT_EQ(0, system(make10MFile.c_str()));
   cleanup.RecalculatePCapDiskUsed(diskUsed_1_MB, totalFiles, pcapDisk);  
   EXPECT_EQ(totalFiles, 1);
   usedMB = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::MB);
   EXPECT_EQ(usedMB, 10);
   EXPECT_EQ(usedMB, diskUsed_1_MB);
   
   cleanup.GetPcapStoreUsage(pcapDisk, DiskUsage::Size::KByte);
   size_t spaceToCreateADirectory = 4;
   EXPECT_EQ(pcapDisk.Used, 10240 + spaceToCreateADirectory);
}
#endif



#ifdef LR_DEBUG

// Could potentially also be DISABLED
// This test will only run if the access to /home and /tmp are on separate
// partitions.
// 
// Mocking so that 'folders' on the same partion is used is not much of a point
// since it is the actual partition calculations and checks that are important
TEST_F(DiskCleanupTest, SystemTest_RecalculatePCapDiskUsedManyPartitions) {
   DiskUsage home("/home/tmp/TooMuchPcap");
   DiskUsage root("/tmp/TooMuchPcap");
   if (home.FileSystemID() != root.FileSystemID()) {
      MockDiskCleanup cleanup(mConf);
      cleanup.mRealFilesSystemAccess = true;
      cleanup.mUseMockConf = true;
      auto& mockedConf = cleanup.mMockedConf;
      tempFileCreate scopedHome(mockedConf, "/home/tmp/TooMuchPcap");
      tempFileCreate scopedRoot(mockedConf, "/tmp/TooMuchPcap");
      ASSERT_TRUE(scopedHome.Init());
      ASSERT_TRUE(scopedRoot.Init());
      mockedConf.mPCapCaptureLocations.clear();
      mockedConf.mPCapCaptureLocations.push_back(scopedHome.mTestDir.str());
      mockedConf.mPCapCaptureLocations.push_back(scopedRoot.mTestDir.str());

      // Verify setup
      ASSERT_EQ(mockedConf.GetPcapCaptureLocations().size(), 2);
      ASSERT_EQ(mockedConf.GetPcapCaptureLocations()[0], scopedHome.mTestDir.str());
      ASSERT_EQ(mockedConf.GetPcapCaptureLocations()[1], scopedRoot.mTestDir.str());
      EXPECT_EQ(mockedConf.GetFirstPcapCaptureLocation(), scopedHome.mTestDir.str());

      Conf& conf = cleanup.GetConf();
      EXPECT_EQ(conf.GetFirstPcapCaptureLocation(), scopedHome.mTestDir.str());
      EXPECT_EQ(conf.GetPcapCaptureLocations()[0], scopedHome.mTestDir.str());
      EXPECT_EQ(conf.GetPcapCaptureLocations()[1], scopedRoot.mTestDir.str());
      EXPECT_NE(scopedRoot.mTestDir.str(), scopedHome.mTestDir.str());
      EXPECT_EQ(mockedConf.GetProbeLocation(), "/usr/local/probe/");

      // Using 2 different partitions for pcaps
      // Verify that RecalculatePCapDiskUsed is using both partitions
      DiskCleanup::DiskSpace pcapDisk{0, 0, 0};
      size_t used_1_a;
      size_t totalFiles;
      cleanup.RecalculatePCapDiskUsed(used_1_a, totalFiles, pcapDisk);
      EXPECT_EQ(totalFiles, 0);
      DiskUsage atRoot{scopedRoot.mTestDir.str()};
      DiskUsage atHome(scopedHome.mTestDir.str());
      auto size = DiskUsage::Size::MB;
      // Measure disk usage before we add anything
      size_t usedMB_1 = atHome.DiskUsed(size);
      size_t usedMB_2 = atRoot.DiskUsed(size);
      EXPECT_EQ(usedMB_1 + usedMB_2, used_1_a);

      // Add 1 file of 10MB to the 'home' partition
      std::string make10MFile = "dd bs=1024 count=10240 if=/dev/zero of=";
      make10MFile += scopedHome.mTestDir.str();
      make10MFile += "/10MFile";
      EXPECT_EQ(0, system(make10MFile.c_str()));

      // Verify that the 10MB file was written to first location
      size_t used_1_b;
      cleanup.RecalculatePCapDiskUsed(used_1_b, totalFiles, pcapDisk);
      EXPECT_EQ(totalFiles, 1);
      DiskCleanup::DiskSpace extraCheck;
      cleanup.GetPcapStoreUsage(extraCheck, size);
      EXPECT_EQ(extraCheck.Used, used_1_b);
      atHome.Update();
      atRoot.Update();
      usedMB_1 = atHome.DiskUsed(size);
      usedMB_2 = atRoot.DiskUsed(size);
      EXPECT_EQ(usedMB_1 + usedMB_2, 10 + used_1_a);
      EXPECT_EQ(usedMB_1 + usedMB_2, used_1_b);


      // Create another 10MB file and save it to the other location
      make10MFile = "dd bs=1024 count=10240 if=/dev/zero of=";
      make10MFile += scopedRoot.mTestDir.str();
      make10MFile += "/10MFile";
      EXPECT_EQ(0, system(make10MFile.c_str()));
      size_t used_1_c;
      cleanup.RecalculatePCapDiskUsed(used_1_c, totalFiles, pcapDisk);
      EXPECT_EQ(totalFiles, 2);
      atHome.Update();
      atRoot.Update();
      usedMB_1 = atHome.DiskUsed(size);
      usedMB_2 = atRoot.DiskUsed(size);
      EXPECT_EQ(usedMB_1 + usedMB_2, 20 + used_1_a);
      EXPECT_EQ(usedMB_1 + usedMB_2, used_1_c);

      // Verify that both files were actually created 
      // in the separate directories and that they are on separate partitions
      std::string path1 = scopedHome.mTestDir.str();
      std::string path2 = scopedRoot.mTestDir.str();
      path1.append("/10MFile");
      path2.append("/10MFile");

      EXPECT_TRUE(boost::filesystem::exists(path1));
      EXPECT_TRUE(boost::filesystem::exists(path2));
      EXPECT_NE(atHome.FileSystemID(), atRoot.FileSystemID());
   }
}
#endif

TEST_F(DiskCleanupTest, CleanupOldPcapFiles) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockDiskCleanup capture(mConf);
      capture.mRealFilesSystemAccess = true;
      SendStats sendQueue;
      DiskCleanup::DiskSpace probeDisk{0, 0, 0};
      DiskCleanup::DiskSpace pcapDisk{100, 0, 0}; // total, free, used

      MockElasticSearch es(false);
      DiskCleanup::PacketCaptureFilesystemDetails previous;
      previous.Writes = 0;
      previous.Reads = 0;
      previous.WriteBytes = 0;
      previous.ReadBytes = 0;
      std::time_t currentTime = std::time(NULL);
      size_t aDiskUsed(0);
      size_t aTotalFiles(0);

      es.mUpdateDocAlwaysPasses = true;
      es.RunQueryGetIdsAlwaysPasses = true;
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      ASSERT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      std::string makeADir;

      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M";
      EXPECT_EQ(0, system(makeSmallFile.c_str())); // 1 file, empty
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);

      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";
      EXPECT_EQ(0, system(make1MFileFile.c_str())); // 2 files, 1MB
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);

      make1MFileFile = "dd bs=1048575 count=1 if=/dev/zero of="; // 1MB-1byte
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1";
      EXPECT_EQ(0, system(make1MFileFile.c_str())); // 3 files, 2MB -1byte (+folder overhead)

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      es.mOldestFiles.emplace_back(testDir.str() + "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M", "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M");
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime,
              aDiskUsed, aTotalFiles, probeDisk, pcapDisk); // empty file removed, 2 left
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.mOldestFiles.clear();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_EQ(2, aTotalFiles);
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup10";
      capture.ResetConf();

      Conf& conf = capture.GetConf();
      auto fileLimit = conf.GetPcapCaptureFileLimit();
      auto sizeLimit = conf.GetPcapCaptureSizeLimit();
      EXPECT_NE(conf.GetPcapCaptureLocations()[0], conf.GetProbeLocation());
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.mOldestFiles.emplace_back(testDir.str() + "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1", "aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1");
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime,
              aDiskUsed, aTotalFiles, probeDisk, pcapDisk); // 2MB files removed
      es.mOldestFiles.clear();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(1, aTotalFiles);
      size_t ByteTotalLeft = 1052672;
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte), ByteTotalLeft);

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_EQ(aTotalFiles, 1);
      EXPECT_EQ(aDiskUsed, 1);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.mOldestFiles.emplace_back(testDir.str() + "/1MFile", "1MFile");
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, probeDisk, pcapDisk);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

   }
#endif
}
#ifdef LR_DEBUG
TEST_F(DiskCleanupTest, ESFailuresGoAheadAndRemoveFiles) {
   if (geteuid() == 0) {
      std::string makeADir;
      SendStats sendQueue;
      DiskCleanup::DiskSpace probeDisk{0, 0, 0};
      DiskCleanup::DiskSpace pcapDisk{100, 0, 0}; // total, free, used
      MockElasticSearch es(false);
      DiskCleanup::PacketCaptureFilesystemDetails previous;
      previous.Writes = 0;
      previous.Reads = 0;
      previous.WriteBytes = 0;
      previous.ReadBytes = 0;
      std::time_t currentTime = std::time(NULL);
      size_t aDiskUsed(0);
      size_t aTotalFiles(0);
      MockDiskCleanup capture(mConf);
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/161122fd-6681-42a3-b953-48beb5247172";
      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      EXPECT_TRUE(boost::filesystem::exists("/tmp/TooMuchPcap/161122fd-6681-42a3-b953-48beb5247172"));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      ASSERT_EQ(mConf.GetConf().GetFirstPcapCaptureLocation(), "/tmp/TooMuchPcap/");
      capture.ResetConf();
      ASSERT_EQ(capture.GetConf().GetFirstPcapCaptureLocation(), "/tmp/TooMuchPcap/");
      ASSERT_EQ(capture.GetConf().GetPcapCaptureLocations()[0], "/tmp/TooMuchPcap/");
      
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_EQ(aTotalFiles, 1);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.mBulkUpdateResult = false;
      es.mOldestFiles.emplace_back(testDir.str() + "/161122fd-6681-42a3-b953-48beb5247172", "161122fd-6681-42a3-b953-48beb5247172");
      size_t filesRemoved;
      size_t spaceSaved;
      time_t oldest;
      EXPECT_EQ(0, capture.RemoveOldestPCapFilesInES(1, es, spaceSaved, oldest));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_FALSE(boost::filesystem::exists("/tmp/TooMuchPcap/161122fd-6681-42a3-b953-48beb5247172"));
      EXPECT_EQ(aTotalFiles, 0);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
     
      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      EXPECT_TRUE(boost::filesystem::exists("/tmp/TooMuchPcap/161122fd-6681-42a3-b953-48beb5247172"));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_EQ(aTotalFiles, 1);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, probeDisk, pcapDisk);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(aTotalFiles, 0);
   }
}

TEST_F(DiskCleanupTest, ESFailuresGoAheadAndRemoveFilesManyLocations) {
   if (geteuid() == 0) {
      std::string makeADir;
      SendStats sendQueue;
      DiskCleanup::DiskSpace probeDisk{0, 0, 0};
      DiskCleanup::DiskSpace pcapDisk{100, 0, 0}; // total, free, used
      MockElasticSearch es(false);
      DiskCleanup::PacketCaptureFilesystemDetails previous;
      previous.Writes = 0;
      previous.Reads = 0;
      previous.WriteBytes = 0;
      previous.ReadBytes = 0;
      std::time_t currentTime = std::time(NULL);
      size_t aDiskUsed(0);
      size_t aTotalFiles(0);
      MockDiskCleanup capture(mConf);
      
      // Fake two capture locations
      EXPECT_EQ(0, system("mkdir -p /tmp/TooMuchPcap/pcap0"));
      EXPECT_EQ(0, system("mkdir -p /tmp/TooMuchPcap/pcap1"));
      
      std::string makeFile1 = "touch ";
      std::string makeFile2 = makeFile1;
      std::string file1 = "/tmp/TooMuchPcap/pcap0/161122fd-6681-42a3-b953-48beb5247172";
      std::string file2 = "/tmp/TooMuchPcap/pcap1/161122fd-6681-42a3-b953-48beb5247174";      
      makeFile1 += file1;
      makeFile2 += file2;
      EXPECT_EQ(0, system(makeFile1.c_str()));
      EXPECT_EQ(0, system(makeFile2.c_str()));
      EXPECT_TRUE(boost::filesystem::exists(file1));
      EXPECT_TRUE(boost::filesystem::exists(file2));
      
      mConf.mConfLocation = "resources/test.yaml.GetPcapStoreUsage1";
      EXPECT_EQ(mConf.GetConf().GetFirstPcapCaptureLocation(), "/tmp/TooMuchPcap/pcap0/");
      capture.ResetConf();
      EXPECT_EQ(capture.GetConf().GetFirstPcapCaptureLocation(), "/tmp/TooMuchPcap/pcap0/");
      auto locations = capture.GetConf().GetPcapCaptureLocations();
      ASSERT_EQ(locations.size(), 2);
      EXPECT_EQ(capture.GetConf().GetPcapCaptureLocations()[0], "/tmp/TooMuchPcap/pcap0/");
      EXPECT_EQ(capture.GetConf().GetPcapCaptureLocations()[1], "/tmp/TooMuchPcap/pcap1/");
      
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_EQ(aTotalFiles, 2);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      es.mBulkUpdateResult = false;
      es.mOldestFiles.emplace_back(file1, "161122fd-6681-42a3-b953-48beb5247172");
      es.mOldestFiles.emplace_back(file2, "161122fd-6681-42a3-b953-48beb52471724");
      size_t filesRemoved;
      size_t spaceSaved;
      time_t oldest;
      EXPECT_EQ(0, capture.RemoveOldestPCapFilesInES(1, es, spaceSaved, oldest));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_FALSE(boost::filesystem::exists(file1));
      EXPECT_FALSE(boost::filesystem::exists(file2));
      EXPECT_EQ(aTotalFiles, 0);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
     
      // Create two files. One in each directory
      // Verify that cleanup works in both directories
      EXPECT_EQ(0, system(makeFile1.c_str()));
      EXPECT_EQ(0, system(makeFile2.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      EXPECT_TRUE(boost::filesystem::exists(file1));
      EXPECT_TRUE(boost::filesystem::exists(file2));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_EQ(aTotalFiles, 2);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, probeDisk, pcapDisk);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDisk);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(aTotalFiles, 0);
      EXPECT_FALSE(boost::filesystem::exists(file1));
      EXPECT_FALSE(boost::filesystem::exists(file2)); 
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
      DiskCleanup::DiskSpace probeDisk{0, 0, 0};
      SendStats sendQueue;
              size_t fsTotalGigs(100);
              MockElasticSearch es(false);
              DiskCleanup::PacketCaptureFilesystemDetails previous;
              previous.Writes = 0;
              previous.Reads = 0;
              previous.WriteBytes = 0;
              previous.ReadBytes = 0;
              std::time_t currentTime = std::time(NULL);
              size_t aDiskUsed(0);
              size_t aTotalFiles(0);

              diskCleanup.CleanupSearch(false, previous, std::ref(es), sendQueue, probeDisk);
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
      DiskCleanup::DiskSpace probeDisk{0, 0, 0};
      SendStats sendQueue;
              size_t fsTotalGigs(100);
              MockElasticSearch es(false);
              DiskCleanup::PacketCaptureFilesystemDetails previous;
              previous.Writes = 0;
              previous.Reads = 0;
              previous.WriteBytes = 0;
              previous.ReadBytes = 0;
              std::time_t currentTime = std::time(NULL);
              size_t aDiskUsed(0);
              size_t aTotalFiles(0);
              diskCleanup.CleanupSearch(false, previous, std::ref(es), sendQueue, probeDisk);

      if (probeDisk.Free != probeDisk.Total) {
         FAIL() << "Not equal as expected: Free = "
                 << probeDisk.Free << ", Total = " << probeDisk.Total;
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

   DiskCleanup::DiskSpace pcapDiskInGB{0, 0, 0};


   diskCleanup.GetPcapStoreUsage(pcapDiskInGB, DiskUsage::Size::GB);
   EXPECT_EQ(100, pcapDiskInGB.Free);
   EXPECT_EQ(109, pcapDiskInGB.Total);

   diskCleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
   diskCleanup.ResetConf();
   diskCleanup.GetPcapStoreUsage(pcapDiskInGB, DiskUsage::Size::GB);
   EXPECT_NE(0, pcapDiskInGB.Free);
   EXPECT_NE(0, pcapDiskInGB.Total);
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
