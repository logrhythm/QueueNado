#include "DiskCleanupTest.h"
#include "DiskCleanup.h"
#include "DiskPacketCapture.h"

#include "SendStats.h"
#include "MockConf.h"
#include "MockElasticSearch.h"
#include <future>
#include <thread>
#include <atomic>

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
      makeSmallFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";  // 1 empty file

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_EQ(aDiskUsed, 0); // 0MB
      EXPECT_EQ(aTotalFiles, 1); 
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles)); // 1 file, limit 1 file
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size:: KByte), 4);
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size:: MB), 0);

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup5";
      capture.ResetConf();
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::MB), 1); // 2 files: 1MB + (4KByte folder overhead)

      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
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
      std::this_thread::sleep_for(std::chrono::seconds(1));
      // 3 files: 2MB + (4KByte folder overhead) - 1byte
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte), 
                                  (2<< B_TO_MB_SHIFT) + (4<<B_TO_KB_SHIFT) - byte); 


      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      size_t filesRemoved;
      size_t spaceSaved;
      capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved);  // 2 files, empty file removed
      EXPECT_EQ(1, filesRemoved);
      EXPECT_EQ(0, spaceSaved);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      // 2MB + (4KByte folder overhead) - 1byte
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte), 
                                  (2<< B_TO_MB_SHIFT) + (4<<B_TO_KB_SHIFT) - byte); 



      mConf.mConfLocation = "resources/test.yaml.DiskCleanup8";
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      EXPECT_EQ(aTotalFiles, 2);
      capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved);
      EXPECT_EQ(1, filesRemoved);
      EXPECT_EQ( 1 << B_TO_MB_SHIFT, spaceSaved); //left is 1 file: 1MB-1byte + (4KByte folder overhead) 
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte), 
                                  (1<< B_TO_MB_SHIFT) + (4<<B_TO_KB_SHIFT) -byte); 
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles)); // 1MB limit vs 1MB-1byte + 4KByte

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9"; // 1MB limit, 1 file limit
      capture.ResetConf();
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.RemoveOldestPCapFiles(1, es, filesRemoved, spaceSaved);// 0 files;
      EXPECT_EQ(1, filesRemoved); // 4KByte folder overhead
      EXPECT_EQ((1<< B_TO_MB_SHIFT) - byte, spaceSaved); // 
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte), 
                                  (0<< B_TO_MB_SHIFT) + (4<<B_TO_KB_SHIFT));

      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
   }
#endif
}

#ifdef LR_DEBUG
TEST_F(DiskCleanupTest, GetPcapStoreUsage) {
   MockDiskCleanup cleanup(mConf);
   cleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
   cleanup.ResetConf();
   Conf copy = cleanup.GetConf();

   EXPECT_EQ(copy.GetPcapCaptureLocation(), "/tmp/TooMuchPcap");
   EXPECT_EQ(copy.GetProbeLocation(), "/tmp/TooMuchPcap");

   // Same partition: disk usage for pcap is calulcated on the folder itself
   // free and total should be huge since it is on the root partition
   size_t free, used, total;
   free = used = total = 0;
   cleanup.GetPcapStoreUsage(free, total, used, DiskUsage::Size::KByte);
   size_t spaceToCreateADirectory = 4;
   EXPECT_EQ(used, spaceToCreateADirectory); // overhead creating dir

   std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
   make1MFileFile += testDir.str();
   make1MFileFile += "/1MFile";
   EXPECT_EQ(0, system(make1MFileFile.c_str()));
   size_t usedKByte = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::KByte);
   EXPECT_EQ(usedKByte, 1024+spaceToCreateADirectory); // including 4: overhead

   cleanup.GetPcapStoreUsage(free, total, used, DiskUsage::Size::KByte);
   EXPECT_EQ(used, 1024+spaceToCreateADirectory);
}
#endif

#ifdef LR_DEBUG
TEST_F(DiskCleanupTest, GetProbeDiskUsage) {
   MockDiskCleanup cleanup(mConf);
   cleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
   cleanup.ResetConf();
   Conf copy = cleanup.GetConf();

   EXPECT_EQ(copy.GetPcapCaptureLocation(), "/tmp/TooMuchPcap");
   EXPECT_EQ(copy.GetProbeLocation(), "/tmp/TooMuchPcap");

   // Same partition: disk usage for pcap is calulcated on the folder itself
   // free and total should be huge since it is on the root partition
   size_t pcapFree=0, pcapTotal = 0, probeFree=0, probeTotal=0;
   std::atomic<size_t> ignored(0);
   std::size_t ignored_2;

   cleanup.GetPcapStoreUsage(pcapFree, pcapTotal, ignored_2, DiskUsage::Size::KByte);
   cleanup.GetProbeFileSystemInfo(probeFree, probeTotal, DiskUsage::Size::KByte);
   
   // Same partition. For "free" and "total" used memory should be the same for 
   //  GetTotal..., GetProbe... and getPcap
   EXPECT_EQ(pcapFree, probeFree);
   EXPECT_EQ(pcapTotal, probeTotal);
   }
#endif



TEST_F(DiskCleanupTest, CleanupOldPcapFiles) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockDiskCleanup capture(mConf);
      capture.mRealFilesSystemAccess = true;
      SendStats sendQueue;
      DiskCleanup::DiskSpaceInGB probeDisk{0,0,0};
      DiskCleanup::DiskSpaceInGB pcapDisk{100,0,0}; // total, free, used
      
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
      EXPECT_EQ(0, system(makeSmallFile.c_str()));  // 1 file, empty
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));  // 2 files, 1MB
      std::this_thread::sleep_for(std::chrono::seconds(1));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      
      make1MFileFile = "dd bs=1048575 count=1 if=/dev/zero of="; // 1MB-1byte
      make1MFileFile += testDir.str();
      make1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbb1Mm1";
      EXPECT_EQ(0, system(make1MFileFile.c_str())); // 3 files, 2MB -1byte (+folder overhead)
      std::this_thread::sleep_for(std::chrono::seconds(1));

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup7";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

      
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, 
              aDiskUsed, aTotalFiles, probeDisk, pcapDisk); // empty file removed, 2 left
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_EQ(2, aTotalFiles);
      mConf.mConfLocation = "resources/test.yaml.DiskCleanup8"; 
      capture.ResetConf();
       
      Conf copy = capture.GetConf();
      auto fileLimit = copy.GetPcapCaptureFileLimit();
      auto sizeLimit = copy.GetPcapCaptureSizeLimit();
      EXPECT_EQ(copy.GetPcapCaptureLocation(), copy.GetProbeLocation());
      EXPECT_TRUE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, 
              aDiskUsed, aTotalFiles, probeDisk, pcapDisk); // 2MB files removed
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles)); 
      EXPECT_EQ(aTotalFiles, 0);
      size_t ByteTotalLeft = (4<<B_TO_KB_SHIFT);// left: 4K folder overhead
      EXPECT_EQ(FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::Byte), ByteTotalLeft);

      mConf.mConfLocation = "resources/test.yaml.DiskCleanup9";
      capture.ResetConf();
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_EQ(0, aTotalFiles);
      EXPECT_EQ(0, aDiskUsed);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles)); 
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
      DiskCleanup::DiskSpaceInGB probeDisk{0,0,0};
      DiskCleanup::DiskSpaceInGB pcapDisk{100,0,0}; // total, free, used
      
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
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, probeDisk, pcapDisk);
      capture.RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
      EXPECT_FALSE(capture.TooMuchPCap(aDiskUsed, aTotalFiles));

   }
}
TEST_F(DiskCleanupTest, ESFailsToQueryRemoveFile) {
   if (geteuid() == 0) {
      std::string makeADir;
      SendStats sendQueue;
      DiskCleanup::DiskSpaceInGB probeDisk{0,0,0};
      DiskCleanup::DiskSpaceInGB pcapDisk{100,0,0}; // total, free, used
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
      capture.CleanupOldPcapFiles(false, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, probeDisk, pcapDisk);
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
      DiskCleanup::DiskSpaceInGB probeDisk{0,0,0};
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
      DiskCleanup::DiskSpaceInGB probeDisk{0,0,0};
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
   auto location = mConf.mConf.GetPcapCaptureLocation();
   diskCleanup.mFleSystemInfo.f_bfree = 100 << B_TO_MB_SHIFT;
   diskCleanup.mFleSystemInfo.f_frsize = 1024;
   diskCleanup.mFleSystemInfo.f_blocks = 109 << B_TO_MB_SHIFT;
   diskCleanup.mFleSystemInfo.f_frsize = 1024;

   size_t free(0);
   size_t total(0);
   size_t used(0);

   diskCleanup.GetPcapStoreUsage(free, total,used, DiskUsage::Size::GB);
   EXPECT_EQ(100, free);
   EXPECT_EQ(109, total);

   diskCleanup.mRealFilesSystemAccess = true;
   mConf.mConfLocation = "resources/test.yaml.DiskCleanup1";
   diskCleanup.ResetConf();
   diskCleanup.GetPcapStoreUsage(free, total,used, DiskUsage::Size::GB);
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
