#pragma once
#include "DiskCleanup.h"
#include <sys/statvfs.h>

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

   void GetFileSystemInfo(size_t& fsFreeGigs, size_t& fsTotalGigs) {
      if (!mFailFileSystemInfo) {
         return DiskCleanup::GetFileSystemInfo(fsFreeGigs, fsTotalGigs);
      }
      if (mFileSystemInfoCountdown-- == 1) {
         fsFreeGigs = fsTotalGigs;
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

   void GetStatVFS(struct statvfs* fileSystemInfo) {
      if (mRealFilesSystemAccess) {
         DiskCleanup::GetStatVFS(fileSystemInfo);
      } else {
         memcpy(fileSystemInfo, &mFleSystemInfo, sizeof (struct statvfs));
      }
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