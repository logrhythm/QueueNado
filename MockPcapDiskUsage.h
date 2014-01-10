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
#include <vector>
#include <string>
//using ::testing::_;
//using ::testing::Invoke;
//using ::testing::Return;
////using ::testing::Throw;
////using ::testing::SetArgReferee;
////using ::testing::DoAll;
//

class MockPcapDiskUsage : public PcapDiskUsage {
   public:
      MockPcapDiskUsage(const std::vector<std::string>& locations, const std::string& programName)
              : PcapDiskUsage(locations, programName), mLocations(locations){}
    
       
    // overrides to enable multiple calls          
    PcapStorage OnceCalculateMountPoints() LR_OVERRIDE{
       return DoCalculateMountPoints(mLocations);
    }          
              
    PcapStorage DoCalculateMountPoints(const std::vector<std::string> locations) LR_OVERRIDE {
       return PcapDiskUsage::DoCalculateMountPoints(locations);
    }
    
    std::vector<std::string> mLocations;
};


class GMockPcapDiskUsage : public MockPcapDiskUsage {
public:
  GMockPcapDiskUsage(const std::vector<std::string>& locations, const std::string& programName)
  : MockPcapDiskUsage(locations, programName), mLocations(locations){}
   
   
  MOCK_METHOD1(GetTotalDiskUsage, PcapDiskUsage(const DiskUsage::Size&));
  MOCK_METHOD0(OnceCalculateMountPoints, PcapDiskUsage());
  MOCK_METHOD1(DoCalculateMountPoints, PcapDiskUsage(const std::vector<std::string>&)); 
};

 

//
//   MOCK_METHOD1(GetTotalDiskUsage, Usage(const DiskUsage::Size&));
//   MOCK_METHOD0(IsSeparatedFromProbeDisk, bool());
//   MOCK_METHOD2(GetDiskUsage, Usage(const std::string&, const DiskUsage::Size&));
//   MOCK_METHOD2(GetFolderUsage, uint64_t(const std::string&, const DiskUsage::Size&));
//   
//   
//   GMockPcapDiskUsage(const std::vector<std::string>& locations, const std::string& probe)
//   : PcapDiskUsage(locations, probe) {
//      
//   }
//   
//   bool Delegate_IsSeparatedFromProbeDisk() { 
//      return PcapDiskUsage::IsSeparatedFromProbeDisk(); 
//   }
//   
//   
//    
//   PcapDiskUsage::Usage Delegate_GetTotalDiskUsage(const DiskUsage::Size& size) { 
//      return PcapDiskUsage::GetTotalDiskUsage(size); 
//   }
//
//
//   //   GMockElasticSearchNoSend(bool async) : MockElasticSearch(async) {
//   //      ON_CALL(*this, SendAndGetReplyCommandToWorker(_, _)).
//   //              WillByDefault(Return(false));
//   //   };
//   //
//   //   GMockElasticSearchNoSend(BoomStick& transport, bool async) : MockElasticSearch(transport, async) {
//   //      ON_CALL(*this, SendAndGetReplyCommandToWorker(_, _)).
//   //              WillByDefault(Return(false));
//   //   };
//   //
//   //   MOCK_METHOD2(SendAndGetReplyCommandToWorker, bool (const std::string& command, std::string& reply));
//
//};
