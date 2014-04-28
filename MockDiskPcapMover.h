#pragma once


#include "DiskPcapMover.h"
#include "include/global.h"
#include "Conf.h"
#include <string>

struct MockDiskPcapMover : public DiskPcapMover {
   MockDiskPcapMover(Conf& conf) 
   : DiskPcapMover(conf)
   , mOverrideProbePcapOriginalLocation(false) 
   {}

   
    bool IsPcapStorageValid(const std::string& location) LR_OVERRIDE {
       return DiskPcapMover::IsPcapStorageValid(location);
    }
    
   bool HasNoUnhashedFiles(const std::string& location) LR_OVERRIDE {
      return DiskPcapMover::HasNoUnhashedFiles(location);
   }
  
    bool DeleteOldPcapStorage() {
       return DiskPcapMover::DeleteOldPcapStorage();
    }
    
    bool HashContentsToPcapBuckets(const std::string& location) LR_OVERRIDE {
       return DiskPcapMover::HashContentsToPcapBuckets(location);
    }
    
   
    bool DoesDirectoryHaveContent(const std::string& location) LR_OVERRIDE {
       return DiskPcapMover::DoesDirectoryHaveContent(location);
    }
    
    bool CreatePcapSubDirectories(const std::string& location) LR_OVERRIDE {
       return DiskPcapMover::CreatePcapSubDirectories(location);
    }
      
   std::string GetOriginalProbePcapLocation() LR_OVERRIDE {
      if (mOverrideProbePcapOriginalLocation) {
         return mConf.GetFirstPcapCaptureLocation();
      }
      return DiskPcapMover::GetOriginalProbePcapLocation();
   }

   bool DirectoryExistsUpToLimit(const std::string& location) LR_OVERRIDE {
      return DiskPcapMover::DirectoryExistsUpToLimit(location);
   }
   
  bool DirectoryNoOverflow(const std::string& location) LR_OVERRIDE {
     return DiskPcapMover::DirectoryNoOverflow(location);
  }
  
   bool FixupPermissions(const std::string& location) LR_OVERRIDE {
      return DiskPcapMover::FixupPermissions(location);
   }
  
   mode_t GetPcapPermissions() const LR_OVERRIDE {
      return DiskPcapMover::GetPcapPermissions();      
   }
   
  bool mOverrideProbePcapOriginalLocation;
};

