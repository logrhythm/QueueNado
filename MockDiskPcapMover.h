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
    
    bool CleanPcapStorageLocation(const std::string& location) LR_OVERRIDE {
       return DiskPcapMover::CleanPcapStorageLocation(location);
    }
   
    bool DoesDirectoryHaveContent(const std::string& location) LR_OVERRIDE {
       return DiskPcapMover::DoesDirectoryHaveContent(location);
    }
    
    bool CleanDirectoryOfFileContents(const std::string& location, size_t& filesRemoved, 
            std::vector<std::string>& foundDirectories) LR_OVERRIDE {
       return DiskPcapMover::CleanDirectoryOfFileContents(location, filesRemoved, foundDirectories);
    }

    bool RemoveEmptyDirectories(const std::vector<std::string>& fullPathDirectories) LR_OVERRIDE {
       return DiskPcapMover::RemoveEmptyDirectories(fullPathDirectories);
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
   
  bool DirectoryNoOverflow(const std::string& location) {
     return DiskPcapMover::DirectoryNoOverflow(location);
  }
  
  bool mOverrideProbePcapOriginalLocation;
};

