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
   MockDiskUsage() 
   : DiskUsage("/usr/local/probe/pcap")
   {
      // snapshot from a statvfs call
      mstatvs.f_bsize = 4096;
      mstatvs.f_frsize = 4096;
      mstatvs.f_blocks = 19051796;
      mstatvs.f_bfree = 15785435;
      mstatvs.f_bavail = 14817653;
      mstatvs.f_files = 4841472;
      mstatvs.f_ffree = 4798215;
      mstatvs.f_favail = 4798215;

      Update();
   }


   bool ReadDiskUsage(struct statvfs& readInto) LR_OVERRIDE {
       readInto = mstatvs;
       return true;
   }
   struct statvfs mstatvs;
   
};
