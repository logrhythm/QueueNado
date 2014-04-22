/* 
 * File:   DiskUsageTest.cpp
 * Author: kjell
 * 
 * Created on September 4, 2013, 10:34 AM
 */

#include "DiskUsageTest.h"
#include "DiskUsage.h"
#include "MockDiskUsage.h"
#include "MockPcapDiskUsage.h"
#include "MockConf.h"
#include "MockProcessClientCommand.h"
#include "include/global.h"
#include "ProcessClient.h"
#include "StopWatch.h"
#include "FileIO.h"
#include <cmath>
using ::testing::_;
using ::testing::Invoke;


namespace {

   struct statvfs GetDefaultMockStatvs() {
      // snapshot from a statvfs call
      struct statvfs toMock;
      toMock.f_bsize = 4096;
      toMock.f_frsize = 4096;
      toMock.f_blocks = 19051796;
      toMock.f_bfree = 15785435;
      toMock.f_bavail = 14817653;
      toMock.f_files = 4841472;
      toMock.f_ffree = 4798215;
      toMock.f_favail = 4798215;
      return toMock;
   }
}
#ifdef LR_DEBUG
TEST(PcapDiskUsage, DoCalculateARealMountPoint) {

   std::vector<std::string> locations{{"/"}, {"/boot"} };
   MockConfMaster confMaster;
   ProcessManager::InstanceWithConfMaster(confMaster);
   ProcessClient processClient(confMaster.GetConf());
   ASSERT_TRUE(processClient.Initialize());
   MockPcapDiskUsage usage(locations, processClient);

   auto storage1 = usage.DoCalculateMountPoints(locations);
   EXPECT_EQ(storage1.size(), 2);
   for (auto& l : storage1) {
      LOG(DEBUG) << l.first << "\tIs a mount point: : " << l.second << std::endl;
   }
   ASSERT_TRUE(storage1.end() != storage1.find("/"));
   ASSERT_TRUE(storage1.end() != storage1.find("/boot/"));
   EXPECT_EQ(true, storage1["/"]);
   // keeping this commented out on purpose. /boot is not necessarily
   // a separate partition
   //  EXPECT_EQ(true, storage1["/boot/"]); 
}
#endif

TEST(DiskUsage, FailedReading) {
   DiskUsage usage("abc");
   auto bytesUsed = usage.DiskUsed(MemorySize::Byte);
   auto bytesTotal = usage.DiskTotal(MemorySize::Byte);
   auto bytesFree = usage.DiskFree(MemorySize::Byte);
   auto usePercentage = usage.DiskUsedPercentage();
   EXPECT_EQ(bytesUsed, 0);
   EXPECT_EQ(bytesTotal, 0);
   EXPECT_EQ(bytesFree, 0);
   EXPECT_EQ(usePercentage, 0);
}
#ifdef LR_DEBUG

TEST(DiskUsage, ReadAtStartup) {
   struct statvfs stat;
   stat.f_bsize = 4096;
   stat.f_frsize = 4096;
   stat.f_blocks = 3000;
   stat.f_bfree = 200;
   stat.f_bavail = 175;
   stat.f_files = 4841472;
   stat.f_ffree = 4798215;
   stat.f_favail = 4798215;

   MockDiskUsage usage(stat);
   usage.Update();
   auto bytesUsed = usage.DiskUsed(MemorySize::Byte);
   auto bytesTotal = usage.DiskTotal(MemorySize::Byte);
   auto bytesFree = usage.DiskFree(MemorySize::Byte);
   auto bytesAvailable = usage.DiskAvailable(MemorySize::Byte);
   double usePercentage = usage.DiskUsedPercentage();
   EXPECT_TRUE(bytesTotal > bytesUsed);
   EXPECT_TRUE(bytesTotal > bytesFree);
   EXPECT_TRUE(bytesTotal > bytesAvailable);

   uint64_t blocks = stat.f_blocks;
   uint64_t block_size = stat.f_frsize;
   uint64_t block_available = stat.f_bavail;
   uint64_t block_free = stat.f_bfree;

   auto sizeInBytes = blocks * block_size;
   auto availableInBytes = block_available * block_size;
   auto diskFreeInBytes = block_free * block_size;
   auto diskUsedInBytes = sizeInBytes - diskFreeInBytes;

   long double used = diskUsedInBytes;
   long double total = diskUsedInBytes + availableInBytes;
   double percent = (100 * used) / total;

   EXPECT_EQ(bytesUsed, diskUsedInBytes);
   EXPECT_EQ(bytesTotal, sizeInBytes);
   EXPECT_EQ(bytesFree, diskFreeInBytes);
   EXPECT_EQ(bytesAvailable, availableInBytes);
   EXPECT_EQ(std::ceil(1000 * usePercentage), std::ceil(1000 * percent));
}

TEST(DiskUsage, ByteToKByteToMBToGB) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   usage.Update();
   auto bytesUsed = usage.DiskUsed(MemorySize::Byte);
   auto bytesTotal = usage.DiskTotal(MemorySize::Byte);
   auto bytesFree = usage.DiskFree(MemorySize::Byte);
   auto bytesAvailable = usage.DiskAvailable(MemorySize::Byte);

   auto KBUsed = usage.DiskUsed(MemorySize::KByte);
   auto KBTotal = usage.DiskTotal(MemorySize::KByte);
   auto KBFree = usage.DiskFree(MemorySize::KByte);
   auto KBAvailable = usage.DiskAvailable(MemorySize::KByte);
   EXPECT_EQ(KBUsed, bytesUsed / 1024);
   EXPECT_EQ(KBTotal, bytesTotal / 1024);
   EXPECT_EQ(KBFree, bytesFree / 1024);
   EXPECT_EQ(KBAvailable, bytesAvailable / 1024);

   auto MBUsed = usage.DiskUsed(MemorySize::MB);
   auto MBTotal = usage.DiskTotal(MemorySize::MB);
   auto MBFree = usage.DiskFree(MemorySize::MB);
   auto MBAvailable = usage.DiskAvailable(MemorySize::MB);
   EXPECT_EQ(MBUsed, KBUsed / 1024); // ceiling adds +1
   EXPECT_EQ(MBTotal, KBTotal / 1024);
   EXPECT_EQ(MBFree, KBFree / 1024);
   EXPECT_EQ(MBAvailable, KBAvailable / 1024);


   auto GBUsed = usage.DiskUsed(MemorySize::GB);
   auto GBTotal = usage.DiskTotal(MemorySize::GB);
   auto GBFree = usage.DiskFree(MemorySize::GB);
   auto GBAvailable = usage.DiskAvailable(MemorySize::GB);
   EXPECT_EQ(GBUsed, MBUsed / 1024);
   EXPECT_EQ(GBTotal, MBTotal / 1024);
   EXPECT_EQ(GBFree, MBFree / 1024);
   EXPECT_EQ(GBAvailable, MBAvailable / 1024);
}

TEST(DiskUsage, PercentageUsed) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   usage.Update();

   double usePercentage = usage.DiskUsedPercentage();
   EXPECT_TRUE(usePercentage > 18) << ". Real percentage was: " << usePercentage;
   EXPECT_TRUE(usePercentage < 19) << ". Real percentage was: " << usePercentage;
}

TEST(DiskUsage, CheckValuesByte) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   auto used = usage.DiskUsed(MemorySize::Byte);
   auto total = usage.DiskTotal(MemorySize::Byte);
   auto free = usage.DiskFree(MemorySize::Byte);
   auto available = usage.DiskAvailable(MemorySize::Byte);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 13379014656);
   EXPECT_EQ(total, 78036156416);
   EXPECT_EQ(free, 64657141760); // free is available + "overhead"
   EXPECT_EQ(available, 60693106688);
   EXPECT_EQ(std::ceil(1000 * percentage), std::ceil(1000 * 18.0621));
}

TEST(DiskUsage, CheckValuesKByte) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   auto used = usage.DiskUsed(MemorySize::KByte);
   auto total = usage.DiskTotal(MemorySize::KByte);
   auto free = usage.DiskFree(MemorySize::KByte);
   auto available = usage.DiskAvailable(MemorySize::KByte);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 13065444);
   EXPECT_EQ(total, 76207184);
   EXPECT_EQ(free, 63141740); // free is available + "overhead"
   EXPECT_EQ(available, 59270612);
   EXPECT_EQ(std::ceil(1000 * percentage), std::ceil(1000 * 18.0621));
}

TEST(DiskUsage, CheckValuesMB) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   auto used = usage.DiskUsed(MemorySize::MB);
   auto total = usage.DiskTotal(MemorySize::MB);
   auto free = usage.DiskFree(MemorySize::MB);
   auto available = usage.DiskAvailable(MemorySize::MB);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 12759);
   EXPECT_EQ(total, 74421);
   EXPECT_EQ(free, 61661); // free is available + "overhead"
   EXPECT_EQ(available, 57881);
   EXPECT_EQ(std::ceil(1000 * percentage), std::ceil(1000 * 18.0621));
}

TEST(DiskUsage, CheckValuesGB) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   auto used = usage.DiskUsed(MemorySize::GB);
   auto total = usage.DiskTotal(MemorySize::GB);
   auto free = usage.DiskFree(MemorySize::GB);
   auto available = usage.DiskAvailable(MemorySize::GB);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 12);
   EXPECT_EQ(total, 72);
   EXPECT_EQ(free, 60);
   EXPECT_EQ(available, 56);
   EXPECT_EQ(std::ceil(1000 * percentage), std::ceil(1000 * 18.0621));
}

#endif

// Under the assumptions  that
//  "/" and "/home" will ALWAYS be on separate
//     disk partitions and "/mnt" will always be on the "/" partition

TEST(DiskUsage, FileSystemID) {
   DiskUsage root("/");
   DiskUsage home("/home");
   DiskUsage mnt("/mnt");
   EXPECT_NE(root.FileSystemID(), home.FileSystemID());
   EXPECT_EQ(root.FileSystemID(), mnt.FileSystemID());
   LOG(INFO) << "\n/home\t\t" << home.FileSystemID()
           << "\n/\t\t" << root.FileSystemID()
           << "\n/mnt\t\t" << mnt.FileSystemID();
}


TEST_F(RaIIFolderUsage, CreateFilesAndCheckSizes_MB) {
   ASSERT_FALSE(FileIO::DoesFileExist({testDir.str() + "/1MFile"}));

   std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
   make1MFileFile += testDir.str();
   make1MFileFile += "/1MFile";
   EXPECT_EQ(0, system(make1MFileFile.c_str()));

   DiskUsage usage(testDir.str());
   size_t usedbyte = usage.RecursiveFolderDiskUsed(testDir.str(), MemorySize::Byte);
   EXPECT_EQ(usedbyte, 1024 * (1024+4)); // only the file, the size of the folder we are "in" overhead

   size_t usedKbyte = usage.RecursiveFolderDiskUsed(testDir.str(), MemorySize::KByte);
   EXPECT_EQ(usedKbyte, 1024+4); 

   size_t usedMB = usage.RecursiveFolderDiskUsed(testDir.str(), MemorySize::MB);
   EXPECT_EQ(usedMB, 1);
}



TEST_F(RaIIFolderUsage, CreateFilesAndCheckSizes_GB) {
   std::string make1GFileFile = "dd bs=1024 count=1048576 if=/dev/zero of=";
   make1GFileFile += testDir.str();
   make1GFileFile += "/1MFile";
   EXPECT_EQ(0, system(make1GFileFile.c_str()));
   DiskUsage usage(testDir.str());
   size_t usedGB = usage.RecursiveFolderDiskUsed(testDir.str(), MemorySize::KByte);
   EXPECT_EQ(usedGB, 1048576 + 4); // 4: overhead?
   usedGB = usage.RecursiveFolderDiskUsed(testDir.str(), MemorySize::MB);
   EXPECT_EQ(usedGB, 1024);
   usedGB = usage.RecursiveFolderDiskUsed(testDir.str(), MemorySize::GB);
   EXPECT_EQ(usedGB, 1);
}
#ifdef LR_DEBUG

TEST(PcapDiskUsage, DoCalculateEmptyMountPoint) {
   MockConf conf;
   std::vector<std::string> locations{
      {""},
      {" "},
      {"           "}
   };
   MockProcessClientCommand processClient(conf);
   ASSERT_TRUE(processClient.Initialize());
   MockPcapDiskUsage usage(locations, processClient);

   auto storage1 = usage.DoCalculateMountPoints(locations);
   EXPECT_EQ(storage1.size(), 0);
}


TEST_F(RaIIFolderUsage, PcapDiskUsage__CalculateFolderUsage__ExpectingOnlyONEFolder) {
   MockConf conf;
   MockProcessClientCommand processClient(conf);
   ASSERT_TRUE(processClient.Initialize());
   GMockPcapDiskUsage pcapUsage({testDir.str()}, processClient);

   // Forward the DoCalculateMountPoints to the real object's DoCalculateMountPoints
   ON_CALL(pcapUsage, DoCalculateMountPoints(_))
           .WillByDefault(Invoke(&pcapUsage, &GMockPcapDiskUsage::CallConcrete__DoCalculateMountPoints));

   ON_CALL(pcapUsage, GetFolderUsage(_, _, _))
           .WillByDefault(Return(100));
   ON_CALL(pcapUsage, GetDiskUsage(_, _))
           .WillByDefault(Return(1000000)); // should not be called

   EXPECT_CALL(pcapUsage, DoCalculateMountPoints(_)).Times(1);
   EXPECT_CALL(pcapUsage, GetFolderUsage(_, _, _)).Times(1);
   EXPECT_CALL(pcapUsage, GetDiskUsage(_, _)).Times(0);

   auto value = pcapUsage.GetTotalDiskUsageRecursive(MemorySize::GB);
   EXPECT_EQ(value.used, 100);
}



TEST_F(RaIIFolderUsage, PcapDiskUsage__CalculateDuplicateFolderUsage__ExpectingOnlyFolder) {
   MockConf conf;
   MockProcessClientCommand processClient(conf);
   ASSERT_TRUE(processClient.Initialize());
   std::string subDirOnSamePartition = testDir.str();
   subDirOnSamePartition.append("/subDir/");
   std::string makeADir = "mkdir -p ";
   makeADir += subDirOnSamePartition;
   system(makeADir.c_str());

   std::vector<std::string> places{testDir.str(), subDirOnSamePartition};
   GMockPcapDiskUsage pcapUsage{places, processClient};

   // Forward the DoCalculateMountPoints to the real object's DoCalculateMountPoints
   ON_CALL(pcapUsage, DoCalculateMountPoints(_))
           .WillByDefault(Invoke(&pcapUsage, &GMockPcapDiskUsage::CallConcrete__DoCalculateMountPoints));

   ON_CALL(pcapUsage, GetFolderUsage(_, _, _))
           .WillByDefault(Return(100));
   ON_CALL(pcapUsage, GetDiskUsage(_, _))
           .WillByDefault(Return(1000000)); // should not be called

   EXPECT_CALL(pcapUsage, DoCalculateMountPoints(_)).Times(1);
   EXPECT_CALL(pcapUsage, GetFolderUsage(_, _, _)).Times(1);
   EXPECT_CALL(pcapUsage, GetDiskUsage(_, _)).Times(0);

   auto value = pcapUsage.GetTotalDiskUsageRecursive(MemorySize::GB);
   EXPECT_EQ(value.used, 100);
}

TEST_F(RaIIFolderUsage, PcapDiskUsage__CalculateDiskUsage__ExpectingOnlyDisk) {
   MockConfMaster confMaster;
   ProcessManager::InstanceWithConfMaster(confMaster);
   ProcessClient processClient(confMaster.GetConf());
   ASSERT_TRUE(processClient.Initialize());
   GMockPcapDiskUsage pcapUsage{
      {"/"}, processClient
   };

   // Forward the DoCalculateMountPoints to the real object's DoCalculateMountPoints
   ON_CALL(pcapUsage, DoCalculateMountPoints(_))
           .WillByDefault(Invoke(&pcapUsage, &GMockPcapDiskUsage::CallConcrete__DoCalculateMountPoints));

   ON_CALL(pcapUsage, GetFolderUsage(_, _, _))
           .WillByDefault(Return(100));
   ON_CALL(pcapUsage, GetDiskUsage(_, _))
           .WillByDefault(Return(1000000)); // should not be called

   EXPECT_CALL(pcapUsage, DoCalculateMountPoints(_)).Times(1);
   EXPECT_CALL(pcapUsage, GetFolderUsage(_, _, _)).Times(0);
   EXPECT_CALL(pcapUsage, GetDiskUsage(_, _)).Times(1); // Only the Disk usage should be called

   auto value = pcapUsage.GetTotalDiskUsageRecursive(MemorySize::GB);
   EXPECT_EQ(value.used, 1000000);
}

/** Calculate disk usage, validate that even when checking a location that is not a 
 * mountpoint we don't call getFolderUsage. getFolderUsage is really slow, we don't want 
 * to run that just to get the total size.
 */
TEST_F(RaIIFolderUsage, PcapDiskUsage__CalculateDiskUsage) {
   MockConfMaster confMaster;
   ProcessManager::InstanceWithConfMaster(confMaster);
   ProcessClient processClient(confMaster.GetConf());
   ASSERT_TRUE(processClient.Initialize());
   GMockPcapDiskUsage pcapUsage{
      {"/usr"}, processClient 
   };

   // Forward the DoCalculateMountPoints to the real object's DoCalculateMountPoints
   ON_CALL(pcapUsage, DoCalculateMountPoints(_))
           .WillByDefault(Invoke(&pcapUsage, &GMockPcapDiskUsage::CallConcrete__DoCalculateMountPoints));

   ON_CALL(pcapUsage, GetFolderUsage(_, _, _))
           .WillByDefault(Return(100));
   ON_CALL(pcapUsage, GetDiskUsage(_, _))
           .WillByDefault(Return(1000000)); // should not be called

   EXPECT_CALL(pcapUsage, DoCalculateMountPoints(_)).Times(1);
   EXPECT_CALL(pcapUsage, GetFolderUsage(_, _, _)).Times(0);
   EXPECT_CALL(pcapUsage, GetDiskUsage(_, _)).Times(1); // Only the Disk usage should be called

   auto value = pcapUsage.GetTotalDiskUsage(MemorySize::GB);
   EXPECT_EQ(value.used, 1000000);
}

TEST_F(RaIIFolderUsage, PcapDiskUsage__CalculateREALFolderUsage) {
   MockConfMaster confMaster;
   ProcessManager::InstanceWithConfMaster(confMaster);
   ProcessClient processClient(confMaster.GetConf());
   ASSERT_TRUE(processClient.Initialize());
   std::string first_1MFileFile = {"dd bs=1024 count=1024 if=/dev/zero of="};
   first_1MFileFile += testDir.str();
   first_1MFileFile += "/aaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb1M";
   EXPECT_EQ(0, system(first_1MFileFile.c_str()));

   std::string second_1MFileFile = {"dd bs=1024 count=1024 if=/dev/zero of="};
   second_1MFileFile += testDir.str();
   second_1MFileFile += "/cccdddddddddddddddddddddddddddddd1M";
   EXPECT_EQ(0, system(second_1MFileFile.c_str()));

   GMockPcapDiskUsage pcapUsage{
      {testDir.str()}, processClient
   };
   // Forward the DoCalculateMountPoints to the real object's DoCalculateMountPoints
   ON_CALL(pcapUsage, DoCalculateMountPoints(_))
           .WillByDefault(Invoke(&pcapUsage, &GMockPcapDiskUsage::CallConcrete__DoCalculateMountPoints));
   ON_CALL(pcapUsage, GetFolderUsage(_, _, _))
           .WillByDefault(Invoke(&pcapUsage, &GMockPcapDiskUsage::CallConcrete__GetFolderUsage));

   ON_CALL(pcapUsage, GetDiskUsage(_, _))
           .WillByDefault(Return(1000000)); // should NOT be called

   EXPECT_CALL(pcapUsage, DoCalculateMountPoints(_)).Times(1);
   EXPECT_CALL(pcapUsage, GetFolderUsage(_, _, _)).Times(1); // Only the folder check should be called
   EXPECT_CALL(pcapUsage, GetDiskUsage(_, _)).Times(0);

   auto value = pcapUsage.GetTotalDiskUsageRecursive(MemorySize::MB);
   EXPECT_EQ(value.used, 2);
}

TEST(DiskUsage, DISABLED_doPrintouts) {
   DiskUsage usage("/home/pcap");
   auto used = usage.DiskUsed(MemorySize::MB);
   auto total = usage.DiskTotal(MemorySize::MB);
   auto free = usage.DiskFree(MemorySize::MB);
   auto available = usage.DiskAvailable(MemorySize::MB);
   auto percentage = usage.DiskUsedPercentage();

   LOG(INFO) << "/home/pcap used: " << used;
   LOG(INFO) << "/home/pcap total: " << total;
   LOG(INFO) << "/home/pcap free: " << free;
   LOG(INFO) << "/home/pcap available: " << available;
   LOG(INFO) << "/home/pcap percentage: " << percentage;
}

TEST(FolderUsage, FolderDoesNotExist) {
   DiskUsage notThere{"abc123"};
   auto result = notThere.RecursiveFolderDiskUsed("abc123", MemorySize::GB);
   EXPECT_EQ(result, 0);
}


TEST_F(RaIIFolderUsage, GetDirectoryDiskUsage__NonExistingDirectory) {
   DiskUsage usage{""};
   auto duByte = usage.RecursiveFolderDiskUsed("/does/not/exist", MemorySize::Byte);
   EXPECT_EQ(duByte, 0);
}

TEST_F(RaIIFolderUsage, GetDirectoryDiskUsage__ExistingEmptyDirectory) {
   DiskUsage usage{""};
   auto path = CreateSubDirectory("empty_dir");
   EXPECT_TRUE(FileIO::DoesDirectoryExist(path));
   auto duByte = usage.RecursiveFolderDiskUsed(path, MemorySize::Byte);
   EXPECT_EQ(duByte, 4096); // the actual empty directory is counted as well
}


TEST_F(RaIIFolderUsage, GetDirectoryDiskUsage__OneLevelWithOneEmptyDirectory) {
   DiskUsage usage{""};
   auto path = CreateSubDirectory("empty_dir1");
   CreateSubDirectory("/empty_dir1/empty_dir2");
   EXPECT_TRUE(FileIO::DoesDirectoryExist(path));
   auto duByte = usage.RecursiveFolderDiskUsed(path, MemorySize::Byte);
   size_t sizeOfDirectory_Byte = 4096;
   EXPECT_EQ(duByte, 2* sizeOfDirectory_Byte);

   size_t sizeOfDirectory_KByte = 4;
   auto duKByte = usage.RecursiveFolderDiskUsed(path, MemorySize::KByte);
   EXPECT_EQ(duKByte, 2 * sizeOfDirectory_KByte);
   
   auto duMB = usage.RecursiveFolderDiskUsed(path, MemorySize::MB);
   EXPECT_EQ(duMB, 0);

   auto duGB = usage.RecursiveFolderDiskUsed(path, MemorySize::GB); 
   EXPECT_EQ(duGB, 0);
}

TEST_F(RaIIFolderUsage, GetDirectoryDiskUsage__OneLevelWithOneFileDirectory) {
   DiskUsage usage{""};
   auto path = CreateSubDirectory("empty_dir1");
   auto path2 = CreateSubDirectory("/empty_dir1/empty_dir2");
   EXPECT_TRUE(FileIO::DoesDirectoryExist(path));
   EXPECT_TRUE(FileIO::DoesDirectoryExist(path2));
   auto filePath = CreateFile(path2, "some_file");
   EXPECT_TRUE(FileIO::DoesFileExist(filePath));

   auto du = usage.RecursiveFolderDiskUsed(path, MemorySize::Byte);
   size_t sizeOfDirectory_Byte = 4096;
   EXPECT_EQ(du, 2* sizeOfDirectory_Byte);
}





// Measure the actual time to do DU recursively on the /usr/local/probe/pcap directory
TEST_F(RaIIFolderUsage, DISABLED_GetDirectoryDiskUsageRealPcapDirectory) {
   std::string path = {"/usr/local/probe/pcap/"};
   DiskUsage usage{path};
   StopWatch watch;
   
   std::string duCmd = {"du -bc " + path + " > /tmp/out.txt"};
   int res = system(duCmd.c_str());
   LOG(INFO) << "****** " << path << " SYSTEM(DU) took: " << watch.ElapsedSec() << " seconds. Result: " << res;

   watch.Restart();
   auto du2 = usage.RecursiveFolderDiskUsed(path, MemorySize::Byte);
   LOG(INFO) << "****** Homemade DU COMMAND ON " << path << " : " << watch.ElapsedSec() << " seconds. Size was: " << du2 << " byte";   

}



TEST_F(RaIIFolderUsage, DISABLED_FolderDoesExist) {
   DiskUsage notThere{"/usr/local/probe/pcap/"};
   auto result_0 = notThere.RecursiveFolderDiskUsed("/usr/local/probe/pcap/", MemorySize::GB);
   EXPECT_TRUE(result_0 > 0);
   LOG(INFO) << "GB usage was: " << result_0;

   auto result_1 = notThere.RecursiveFolderDiskUsed("/usr/local/probe/pcap", MemorySize::GB);
   EXPECT_TRUE(result_1 >= result_0);
   LOG(INFO) << "GB usage was: " << result_1;
}

#endif
