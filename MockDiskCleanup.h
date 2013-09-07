#pragma once
#include "DiskCleanup.h"
#include <sys/statvfs.h>
#include <MockDiskUsage.h>

class MockDiskCleanup : public DiskCleanup {
public:

   MockDiskCleanup(networkMonitor::ConfSlave& conf) : DiskCleanup(conf), mFailRemoveSearch(false),
   mFailFileSystemInfo(false), mFileSystemInfoCountdown(0), mSucceedRemoveSearch(false),
   mRealFilesSystemAccess(false) {
      mFleSystemInfo.f_bfree = 1;
      mFleSystemInfo.f_frsize = 1;
      mFleSystemInfo.f_blocks = 1;
      mFleSystemInfo.f_frsize = 1;
   }

   virtual ~MockDiskCleanup() {
   }

   bool IsShutdown() {
      return false;
   }
   size_t RemoveOldestPCapFiles(const size_t maxToRemove, ElasticSearch& es, size_t& filesRemoved, size_t& spaceRemoved) {
      return DiskCleanup::RemoveOldestPCapFiles(1, es, filesRemoved, spaceRemoved);
   }

   bool TooMuchPCap(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      return DiskCleanup::TooMuchPCap(aDiskUsed, aTotalFiles);
   }

   void RecalculatePCapDiskUsed(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      DiskCleanup::RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
   }

   void CleanupOldPcapFiles(bool canSendStats, PacketCaptureFilesystemDetails& previous, ElasticSearch& es, SendStats& sendQueue,
           std::time_t& currentTime, std::atomic<size_t>& aDiskUsed,
           std::atomic<size_t>& aTotalFiles,
           const size_t fsFreeGigs,
           const size_t fsTotalGigs) {
      DiskCleanup::CleanupOldPcapFiles(canSendStats, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
   }

   bool TooMuchSearch(const size_t& fsFreeGigs, const size_t& fsTotalGigs) {
      return DiskCleanup::TooMuchSearch(fsFreeGigs, fsTotalGigs);
   }

   void ResetConf() {
      DiskCleanup::ResetConf();
   }

   bool RemoveOldestSearchIndex(ElasticSearch& es) {
      if (mSucceedRemoveSearch) {
         return true;
      }
      if (!mFailRemoveSearch) {
         return DiskCleanup::RemoveOldestSearchIndex(es);
      }


      return false;
   }

   // TODO pcapture or probe location?size_t& fsFree, size_t& fsTotal, const DiskUsage::Size size
   void GetPcapStoreUsage(size_t& fsFree, size_t& fsTotal, size_t& fsUsed, const DiskUsage::Size size) {
      if (!mFailFileSystemInfo) {
         if (mRealFilesSystemAccess) {
            DiskCleanup::GetPcapStoreUsage(fsFree, fsTotal, fsUsed, size);
         }  else {
            struct statvfs mockStatvs;
            mockStatvs.f_bsize = mFleSystemInfo.f_bsize;
            mockStatvs.f_frsize = mFleSystemInfo.f_frsize;
            mockStatvs.f_blocks = mFleSystemInfo.f_blocks;
            mockStatvs.f_bfree = mFleSystemInfo.f_bfree;
            mockStatvs.f_bavail = 1;
            mockStatvs.f_files = 1;
            mockStatvs.f_ffree = 1;
            mockStatvs.f_favail = 1;
            MockDiskUsage disk(mockStatvs);

            disk.Update();
            fsFree = disk.DiskFree(size);
             fsTotal = disk.DiskTotal(size); 
         }
      }
      if (mFileSystemInfoCountdown-- == 1) {
         fsFree = fsTotal;
      }
      return;
   }

   void CleanupSearch(bool canSendStats, PacketCaptureFilesystemDetails& previous, ElasticSearch& es, SendStats& sendQueue,
           std::time_t& currentTime, const std::atomic<size_t>& aDiskUsed,
           const std::atomic<size_t>& aTotalFiles,
           size_t& fsFreeGigs,
           size_t& fsTotalGigs) {
      return DiskCleanup::CleanupSearch(canSendStats, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, fsFreeGigs, fsTotalGigs);
   }


   std::string GetOldestIndex(ElasticSearch& es) {
      return DiskCleanup::GetOldestIndex(es);
   }
   bool mFailRemoveSearch;
   bool mFailFileSystemInfo;
   int mFileSystemInfoCountdown;
   bool mSucceedRemoveSearch;
   struct statvfs mFleSystemInfo;
   bool mRealFilesSystemAccess;
};
