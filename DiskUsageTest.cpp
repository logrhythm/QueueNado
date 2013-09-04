/* 
 * File:   DiskUsageTest.cpp
 * Author: kjell
 * 
 * Created on September 4, 2013, 10:34 AM
 */

#include "DiskUsageTest.h"
#include "DiskUsage.h"
#include "MockDiskUsage.h"



TEST(DiskUsage, InstantiationPreDiskReading) {
   bool mockStats = true;
   MockDiskUsage usage("/home", mockStats);
   auto bytesUsed = usage.DiskUsed(DiskUsage::Size::Byte);
   auto bytesTotal = usage.DiskTotal(DiskUsage::Size::Byte);
   auto bytesFree = usage.DiskFree(DiskUsage::Size::Byte);
   auto usePercentage = usage.DiskUsedPercentage();
   EXPECT_EQ(bytesUsed, 0);
   EXPECT_EQ(bytesTotal, 0);
   EXPECT_EQ(bytesFree, 0);
   EXPECT_EQ(usePercentage, 0);
}

TEST(DiskUsage, InstantiationPostDiskReading) {
   bool mockStats = true;
   MockDiskUsage usage("/home", mockStats);
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
   bool mockStats = true;
   MockDiskUsage usage("/home", mockStats);
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
   EXPECT_EQ(MBUsed, KBUsed/1024);
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
   bool mockStats = true;
   MockDiskUsage usage("/home", mockStats);
   usage.Update();

   double usePercentage = usage.DiskUsedPercentage();
   EXPECT_TRUE(usePercentage > 18) << ". Real percentage was: " << usePercentage;
   EXPECT_TRUE(usePercentage < 19) << ". Real percentage was: " << usePercentage;
}