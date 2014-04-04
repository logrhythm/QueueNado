#pragma once


#include "DiskPcapMover.h"
#include "include/global.h"
#include "Conf.h"
#include <string>

struct MockDiskPcapMover : public DiskPcapMover {
   MockDiskPcapMover(Conf& conf) : DiskPcapMover(conf) {}

      
   std::string GetProbePcapLocation() LR_OVERRIDE {
      return DiskPcapMover::GetProbePcapLocation();
   }

   bool DirectoryExistsUpToLimit(const std::string& location) LR_OVERRIDE {
      return DiskPcapMover::DirectoryExistsUpToLimit(location);
   }
   
  bool DirectoryNoOverflow(const std::string& location) {
     return DiskPcapMover::DirectoryNoOverflow(location);
  }
};

