/* 
 * File:   MockDiskUsage.h
 * Author: kjell
 *
 * Created on September 4, 2013, 10:37 AM
 */

#pragma once

#include "DiskUsage.h"
#include <sys/statvfs.h>
#include <g2log.hpp>

struct MockDiskUsage : public DiskUsage {
   MockDiskUsage(const struct statvfs& mockStatvs,const std::string& programName) 
   : DiskUsage("/usr/local/probe/pcap",programName)
   , mstatvs(mockStatvs)
   {
      Update();
   }


   bool ReadDiskUsage(struct statvfs& readInto) LR_OVERRIDE {
       readInto = mstatvs;
       return true;
   }
   struct statvfs mstatvs;
   
};
