/* 
 * File:   DiskUsageTest.cpp
 * Author: kjell
 * 
 * Created on September 4, 2013, 10:34 AM
 */

#include "DiskUsageTest.h"
#include "DiskUsage.h"
#include "MockDiskUsage.h"
#include "Conf.h"
#include <cmath>


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



TEST(DiskUsage, FailedReading) {
   DiskUsage usage("abc");
   auto bytesUsed = usage.DiskUsed(DiskUsage::Size::Byte);
   auto bytesTotal = usage.DiskTotal(DiskUsage::Size::Byte);
   auto bytesFree = usage.DiskFree(DiskUsage::Size::Byte);
   auto usePercentage = usage.DiskUsedPercentage();
   EXPECT_EQ(bytesUsed, 0);
   EXPECT_EQ(bytesTotal, 0);
   EXPECT_EQ(bytesFree, 0);
   EXPECT_EQ(usePercentage, 0);
}

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
   auto bytesUsed = usage.DiskUsed(DiskUsage::Size::Byte);
   auto bytesTotal = usage.DiskTotal(DiskUsage::Size::Byte);
   auto bytesFree = usage.DiskFree(DiskUsage::Size::Byte);
   auto bytesAvailable = usage.DiskAvailable(DiskUsage::Size::Byte);
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
   EXPECT_EQ(std::ceil(1000* usePercentage), std::ceil(1000*percent));
}


TEST(DiskUsage, ByteToKByteToMBToGB) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   usage.Update();
   auto bytesUsed = usage.DiskUsed(DiskUsage::Size::Byte);
   auto bytesTotal = usage.DiskTotal(DiskUsage::Size::Byte);
   auto bytesFree = usage.DiskFree(DiskUsage::Size::Byte);
   auto bytesAvailable = usage.DiskAvailable(DiskUsage::Size::Byte);

   auto KBUsed = usage.DiskUsed(DiskUsage::Size::KByte);
   auto KBTotal = usage.DiskTotal(DiskUsage::Size::KByte);
   auto KBFree = usage.DiskFree(DiskUsage::Size::KByte);
   auto KBAvailable = usage.DiskAvailable(DiskUsage::Size::KByte);
   EXPECT_EQ(KBUsed, bytesUsed/1024);
   EXPECT_EQ(KBTotal, bytesTotal/1024);
   EXPECT_EQ(KBFree, bytesFree/1024);
   EXPECT_EQ(KBAvailable, bytesAvailable/1024);
   
   auto MBUsed = usage.DiskUsed(DiskUsage::Size::MB);
   auto MBTotal = usage.DiskTotal(DiskUsage::Size::MB);
   auto MBFree = usage.DiskFree(DiskUsage::Size::MB);
   auto MBAvailable = usage.DiskAvailable(DiskUsage::Size::MB);
   EXPECT_EQ(MBUsed, KBUsed/1024); // ceiling adds +1
   EXPECT_EQ(MBTotal, KBTotal/1024);
   EXPECT_EQ(MBFree, KBFree/1024);
   EXPECT_EQ(MBAvailable, KBAvailable/1024);
   
   
   auto GBUsed = usage.DiskUsed(DiskUsage::Size::GB);
   auto GBTotal = usage.DiskTotal(DiskUsage::Size::GB);
   auto GBFree = usage.DiskFree(DiskUsage::Size::GB);
   auto GBAvailable = usage.DiskAvailable(DiskUsage::Size::GB);
   EXPECT_EQ(GBUsed, MBUsed/1024);
   EXPECT_EQ(GBTotal, MBTotal/1024);
   EXPECT_EQ(GBFree, MBFree/1024);
   EXPECT_EQ(GBAvailable, MBAvailable/1024);
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
   auto used = usage.DiskUsed(DiskUsage::Size::Byte);
   auto total = usage.DiskTotal(DiskUsage::Size::Byte);
   auto free = usage.DiskFree(DiskUsage::Size::Byte);
   auto available = usage.DiskAvailable(DiskUsage::Size::Byte);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 13379014656);
   EXPECT_EQ(total, 78036156416);
   EXPECT_EQ(free, 64657141760); // free is available + "overhead"
   EXPECT_EQ(available, 60693106688);
   EXPECT_EQ(std::ceil(1000* percentage), std::ceil(1000*18.0621));
}


TEST(DiskUsage, CheckValuesKByte) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   auto used = usage.DiskUsed(DiskUsage::Size::KByte);
   auto total = usage.DiskTotal(DiskUsage::Size::KByte);
   auto free = usage.DiskFree(DiskUsage::Size::KByte);
   auto available = usage.DiskAvailable(DiskUsage::Size::KByte);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 13065444);
   EXPECT_EQ(total, 76207184);
   EXPECT_EQ(free, 63141740); // free is available + "overhead"
   EXPECT_EQ(available, 59270612);
   EXPECT_EQ(std::ceil(1000* percentage), std::ceil(1000*18.0621));
}


TEST(DiskUsage, CheckValuesMB) {

   MockDiskUsage usage(GetDefaultMockStatvs());
   auto used = usage.DiskUsed(DiskUsage::Size::MB);
   auto total = usage.DiskTotal(DiskUsage::Size::MB);
   auto free = usage.DiskFree(DiskUsage::Size::MB);
   auto available = usage.DiskAvailable(DiskUsage::Size::MB);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 12759);
   EXPECT_EQ(total, 74421);
   EXPECT_EQ(free, 61661); // free is available + "overhead"
   EXPECT_EQ(available, 57881);
   EXPECT_EQ(std::ceil(1000* percentage), std::ceil(1000*18.0621));
} 


TEST(DiskUsage, CheckValuesGB) {
   MockDiskUsage usage(GetDefaultMockStatvs());
   auto used = usage.DiskUsed(DiskUsage::Size::GB);
   auto total = usage.DiskTotal(DiskUsage::Size::GB);
   auto free = usage.DiskFree(DiskUsage::Size::GB);
   auto available = usage.DiskAvailable(DiskUsage::Size::GB);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 12);
   EXPECT_EQ(total, 72);
   EXPECT_EQ(free, 60);
   EXPECT_EQ(available, 56);
   EXPECT_EQ(std::ceil(1000* percentage), std::ceil(1000*18.0621));
}



// Under the assumptions  that
//  "/" and "/home" will ALWAYS be on separate
//     disk partitions and "/mnt" will always be on the "/" partition
TEST(DiskUsage, FileSystemID) {
  DiskUsage root{"/"};
  DiskUsage home{"/home"};
  DiskUsage mnt{"/mnt"};
  EXPECT_NE(root.FileSystemID(), home.FileSystemID());
  EXPECT_EQ(root.FileSystemID(), mnt.FileSystemID());
  LOG(INFO) << "\n/home\t\t" << home.FileSystemID() 
            << "\n/\t\t" << root.FileSystemID()
            << "\n/mnt\t\t" << mnt.FileSystemID();
}





TEST_F(RaIIFolderUsage, CreateFilesAndCheckSizes_MB) {
   std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
   make1MFileFile += testDir.str();
   make1MFileFile += "/1MFile";
   EXPECT_EQ(0, system(make1MFileFile.c_str()));

   size_t usedMB = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::KByte);
   EXPECT_EQ(usedMB, 1024+4); // overhead
   usedMB = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::MB);
   EXPECT_EQ(usedMB, 1);
}


TEST_F(RaIIFolderUsage, CreateFilesAndCheckSizes_GB) {
   std::string make1GFileFile = "dd bs=1024 count=1048576 if=/dev/zero of=";
   make1GFileFile += testDir.str();
   make1GFileFile += "/1MFile";
   EXPECT_EQ(0, system(make1GFileFile.c_str()));

   size_t usedGB = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::KByte);
   EXPECT_EQ(usedGB, 1048576+4); // 4: overhead?
   usedGB = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::MB);
   EXPECT_EQ(usedGB, 1024);
   usedGB = FolderUsage::DiskUsed(testDir.str(), DiskUsage::Size::GB);
   EXPECT_EQ(usedGB, 1);
}


TEST(DiskUsage, DISABLED_doPrintouts) {
 
   DiskUsage usage("/home/pcap");
   auto used = usage.DiskUsed(DiskUsage::Size::MB);
   auto total = usage.DiskTotal(DiskUsage::Size::MB);
   auto free = usage.DiskFree(DiskUsage::Size::MB);
   auto available = usage.DiskAvailable(DiskUsage::Size::MB);
   auto percentage = usage.DiskUsedPercentage();

   LOG(INFO) << "/home/pcap used: " << used;
   LOG(INFO) << "/home/pcap total: " << total;
   LOG(INFO) << "/home/pcap free: " << free;
   LOG(INFO) << "/home/pcap available: " << available;
   LOG(INFO) << "/home/pcap percentage: " << percentage;
}

// df and DiskUsage give very similar answers
// du and FolderUsage give very similar answers
// BUT: df and du differ in answer with about 5.8% or more ?? with 
// df giving the higher answer
TEST(DiskUsage, DISABLED_ToWaysToCheck) {
  DiskUsage home{"/home/"};
  auto homeUsed = home.DiskUsed(DiskUsage::Size::KByte);
  auto homeAsFolder = FolderUsage::DiskUsed("/home/", DiskUsage::Size::KByte);
  
  ASSERT_GE(homeUsed, homeAsFolder);
  size_t percentUnitsx10 = (1000* (homeUsed - homeAsFolder))/homeAsFolder;  
  EXPECT_EQ(percentUnitsx10, 58);
}

TEST(FolderUsage, FolderDoesNotExist) {
    auto result = FolderUsage::DiskUsed("abc123", DiskUsage::Size::GB);
    EXPECT_EQ(result, 0);
}



TEST(FolderUsage, DISABLED_FolderDoesExist) {
    auto result_0 = FolderUsage::DiskUsed("/usr/local/probe/pcap/", DiskUsage::Size::GB);
    EXPECT_TRUE(result_0 > 0);
    LOG(INFO) << "GB usage was: " << result_0;
    
    auto result_1 = FolderUsage::DiskUsed("/usr/local/probe/pcap", DiskUsage::Size::GB);
    EXPECT_TRUE(result_1 >= result_0);
    LOG(INFO) << "GB usage was: " << result_1;
}



