
#include "DriveInfo.h"
#include "gmock/gmock.h"

class MockDriveInfo : public DriveInfo {
public:

   explicit MockDriveInfo(const std::string& parted)
   : DriveInfo(parted) {
   }

   bool BuildDriveInfo(const std::string& deviceDetails) LR_OVERRIDE {
      return DriveInfo::BuildDriveInfo(deviceDetails);
   }

   bool BuildPartitionInfo(const std::string& partitionDetails) LR_OVERRIDE {
      return DriveInfo::BuildPartitionInfo(partitionDetails);
   }
};

