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

   void RemoveOldestPCapFiles(const size_t maxToRemove) override {
      DiskCleanup::RemoveOldestPCapFiles(1);
   }

   bool TooMuchPCap(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) override {
      return DiskCleanup::TooMuchPCap(aDiskUsed, aTotalFiles);
   }

   void RecalculatePCapDiskUsed(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) override {
      DiskCleanup::RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles);
   }

   void CleanupOldPcapFiles(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) override {
      DiskCleanup::CleanupOldPcapFiles(aDiskUsed, aTotalFiles);
   }

   bool TooMuchSearch(const size_t& fsFreeGigs, const size_t& fsTotalGigs) override {
      return DiskCleanup::TooMuchSearch(fsFreeGigs, fsTotalGigs);
   }

   void ResetConf() override {
      DiskCleanup::ResetConf();
   }

   bool RemoveOldestSearchIndex() override {
      if (mSucceedRemoveSearch) {
         return true;
      }
      if (!mFailRemoveSearch) {
         return DiskCleanup::RemoveOldestSearchIndex();
      }


      return false;
   }

   void GetFileSystemInfo(size_t& fsFreeGigs, size_t& fsTotalGigs) override {
      if (!mFailFileSystemInfo) {
         return DiskCleanup::GetFileSystemInfo(fsFreeGigs, fsTotalGigs);
      }
      if (mFileSystemInfoCountdown-- == 1) {
         fsFreeGigs = fsTotalGigs;
      }
      return;
   }

   void CleanupSearch(size_t& fsFreeGigs, size_t& fsTotalGigs) override {
      return DiskCleanup::CleanupSearch(fsFreeGigs, fsTotalGigs);
   }

   void GetStatVFS(struct statvfs* fileSystemInfo) {
      if (mRealFilesSystemAccess) {
         DiskCleanup::GetStatVFS(fileSystemInfo);
      } else {
         memcpy(fileSystemInfo, &mFleSystemInfo, sizeof (struct statvfs));
      }
   }
   bool mFailRemoveSearch;
   bool mFailFileSystemInfo;
   int mFileSystemInfoCountdown;
   bool mSucceedRemoveSearch;
   struct statvfs mFleSystemInfo;
   bool mRealFilesSystemAccess;
};