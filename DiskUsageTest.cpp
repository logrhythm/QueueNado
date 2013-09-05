/* 
 * File:   DiskUsageTest.cpp
 * Author: kjell
 * 
 * Created on September 4, 2013, 10:34 AM
 */

#include "DiskUsageTest.h"
#include "DiskUsage.h"
#include "MockDiskUsage.h"
#include <cmath>
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
   MockDiskUsage usage;
   usage.Update();
   auto bytesUsed = usage.DiskUsed(DiskUsage::Size::Byte);
   auto bytesTotal = usage.DiskTotal(DiskUsage::Size::Byte);
   auto bytesFree = usage.DiskFree(DiskUsage::Size::Byte);
   auto bytesAvailable = usage.DiskAvailable(DiskUsage::Size::Byte);
   double usePercentage = usage.DiskUsedPercentage();
   EXPECT_TRUE(bytesTotal > bytesUsed);
   EXPECT_TRUE(bytesTotal > bytesFree);
   EXPECT_TRUE(bytesTotal > bytesAvailable);
   
   EXPECT_TRUE(bytesUsed > 0);
   EXPECT_TRUE(bytesTotal > 0);
   EXPECT_TRUE(bytesFree > 0);
   EXPECT_TRUE(bytesAvailable > 0);
   EXPECT_TRUE(usePercentage > 0);
}


TEST(DiskUsage, ByteToKByteToMBToGB) {
   MockDiskUsage usage;
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
   EXPECT_EQ(MBUsed, 1 + KBUsed/1024); // ceiling adds +1
   EXPECT_EQ(MBTotal, 1 + KBTotal/1024);
   EXPECT_EQ(MBFree, 1 +KBFree/1024);
   EXPECT_EQ(MBAvailable, 1 +KBAvailable/1024);
   
   
   auto GBUsed = usage.DiskUsed(DiskUsage::Size::GB);
   auto GBTotal = usage.DiskTotal(DiskUsage::Size::GB);
   auto GBFree = usage.DiskFree(DiskUsage::Size::GB);
   auto GBAvailable = usage.DiskAvailable(DiskUsage::Size::GB);
   EXPECT_EQ(GBUsed, 1 + MBUsed/1024);
   EXPECT_EQ(GBTotal, 1 + MBTotal/1024);
   EXPECT_EQ(GBFree, 1 + MBFree/1024);
   EXPECT_EQ(GBAvailable, 1 + MBAvailable/1024);
}


TEST(DiskUsage, PercentageUsed) {
   MockDiskUsage usage;
   usage.Update();

   double usePercentage = usage.DiskUsedPercentage();
   EXPECT_TRUE(usePercentage > 18) << ". Real percentage was: " << usePercentage;
   EXPECT_TRUE(usePercentage < 19) << ". Real percentage was: " << usePercentage;
}


TEST(DiskUsage, CheckValuesByte) {
   MockDiskUsage usage;
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
   EXPECT_EQ(std::ceil(percentage), std::ceil(18.80));
}


TEST(DiskUsage, CheckValuesKByte) {
   MockDiskUsage usage;
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
   EXPECT_EQ(std::ceil(percentage), std::ceil(18.80));
}


TEST(DiskUsage, CheckValuesMB) {

   MockDiskUsage usage;
   auto used = usage.DiskUsed(DiskUsage::Size::MB);
   auto total = usage.DiskTotal(DiskUsage::Size::MB);
   auto free = usage.DiskFree(DiskUsage::Size::MB);
   auto available = usage.DiskAvailable(DiskUsage::Size::MB);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 12760);
   EXPECT_EQ(total, 74422);
   EXPECT_EQ(free, 61662); // free is available + "overhead"
   EXPECT_EQ(available, 57882);
   EXPECT_EQ(std::ceil(percentage), std::ceil(18.80));
} 


TEST(DiskUsage, CheckValuesGB) {
   MockDiskUsage usage;
   auto used = usage.DiskUsed(DiskUsage::Size::GB);
   auto total = usage.DiskTotal(DiskUsage::Size::GB);
   auto free = usage.DiskFree(DiskUsage::Size::GB);
   auto available = usage.DiskAvailable(DiskUsage::Size::GB);
   auto percentage = usage.DiskUsedPercentage();

   // from snapshot 
   EXPECT_EQ(used, 13);
   EXPECT_EQ(total, 73);
   EXPECT_EQ(free, 61);
   EXPECT_EQ(available, 57);
   EXPECT_EQ(std::ceil(percentage), std::ceil(18.79));
}


#if 0
TEST(DiskUsage, doPrintouts) {
 
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
#endif 


