///* 
// * File:   MockPcapDiskUsage.h
// * Author: kjell
// *
// * Created on December 16, 2013, 12:34 PM
// */
//
#pragma once
#include "gmock/gmock.h"
#include "PcapDiskUsage.h"
#include "include/global.h"
#include <vector>
#include <string>
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

////using ::testing::Throw;
////using ::testing::SetArgReferee;
////using ::testing::DoAll;
//

class MockPcapDiskUsage : public PcapDiskUsage {
public:

   MockPcapDiskUsage(const std::vector<std::string>& locations)
   : PcapDiskUsage(locations), mLocations(locations) {
   }

   PcapStorage DoCalculateMountPoints(const std::vector<std::string>& locations) LR_OVERRIDE {
      return PcapDiskUsage::DoCalculateMountPoints(locations);
   }

   size_t GetDiskUsage(DiskUsage& disk, const MemorySize& size) LR_OVERRIDE {
      return PcapDiskUsage::GetDiskUsage(disk, size);
   }

   size_t GetFolderUsage(const std::string& path, DiskUsage& disk, const MemorySize& size) LR_OVERRIDE {
      return PcapDiskUsage::GetFolderUsage(path, disk, size);
   }
   std::vector<std::string> mLocations;
};

class GMockPcapDiskUsage : public MockPcapDiskUsage {
public:

   GMockPcapDiskUsage(const std::vector<std::string>& locations)
   : MockPcapDiskUsage(locations) {
   }

   MOCK_METHOD1(DoCalculateMountPoints, PcapStorage(const std::vector<std::string>&));
   MOCK_METHOD2(GetDiskUsage, size_t(DiskUsage&, const MemorySize&));
   MOCK_METHOD3(GetFolderUsage, size_t(const std::string&, DiskUsage&, const MemorySize&));
   
   
   void DelegateFolderUsage(size_t value) {
      EXPECT_CALL(*this, GetFolderUsage(_,_,_))
              .WillRepeatedly(Return(value));
   }
   
   PcapStorage CallConcrete__DoCalculateMountPoints(const std::vector<std::string>& loc) {
      return MockPcapDiskUsage::DoCalculateMountPoints(loc);
   }
   
   size_t CallConcrete__GetFolderUsage(const std::string& path, DiskUsage& disk, const MemorySize& size) {
      return MockPcapDiskUsage::GetFolderUsage(path, disk, size);
   }
};