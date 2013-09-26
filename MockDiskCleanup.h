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

   size_t RemoveOldestPCapFilesInES(const size_t maxToRemove, ElasticSearch& es, size_t& spaceSaved, time_t& oldest) {
      return DiskCleanup::RemoveOldestPCapFilesInES(1, es, spaceSaved, oldest);
   }

   bool TooMuchPCap(size_t& aDiskUsed, size_t& aTotalFiles) {
      return DiskCleanup::TooMuchPCap(aDiskUsed, aTotalFiles);
   }

   void RecalculatePCapDiskUsed(size_t& aDiskUsed, size_t& aTotalFiles, DiskSpace& pcapDiskInGB) {
      DiskCleanup::RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDiskInGB);
   }

   void CleanupOldPcapFiles(bool canSendStats, PacketCaptureFilesystemDetails& previous, ElasticSearch& es, SendStats& sendQueue,
           std::time_t& currentTime, size_t& aDiskUsed,
           size_t& aTotalFiles,
        const DiskSpace& probeDiskInGB,
        DiskSpace& pcapDiskInGB) {
      DiskCleanup::CleanupOldPcapFiles(canSendStats, previous, es, sendQueue, currentTime, aDiskUsed, aTotalFiles, probeDiskInGB, pcapDiskInGB);
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

   void GetPcapStoreUsage(DiskSpace& pcapDiskInGB, 
                          const DiskUsage::Size size) {
         if (mRealFilesSystemAccess) {
            DiskCleanup::GetPcapStoreUsage(pcapDiskInGB, size);
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
            pcapDiskInGB.Free = disk.DiskFree(size);
            pcapDiskInGB.Total = disk.DiskTotal(size); 
            pcapDiskInGB.Used = disk.DiskUsed(size); 
         }
   }

   void GetProbeFileSystemInfo(DiskSpace& probeDiskInGB, 
                               const DiskUsage::Size size) {
      if (!mFailFileSystemInfo) {
         if (mRealFilesSystemAccess) {
            DiskCleanup::GetProbeFileSystemInfo(probeDiskInGB,size);
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
            probeDiskInGB.Free = disk.DiskFree(size);
            probeDiskInGB.Total = disk.DiskTotal(size); 
            probeDiskInGB.Used = disk.DiskUsed(size); 
         }
      }
      if (mFileSystemInfoCountdown-- == 1) {
         probeDiskInGB.Free = probeDiskInGB.Total;
      }
      return;
   }

   void CleanupSearch(bool canSendStats, PacketCaptureFilesystemDetails& previous, 
           ElasticSearch& es, SendStats& sendQueue, DiskSpace& probeDiskUsage) {
      return DiskCleanup::CleanupSearch(canSendStats, previous, es, sendQueue, probeDiskUsage);
   }


   std::string GetOldestIndex(ElasticSearch& es) {
      return DiskCleanup::GetOldestIndex(es);
   }


   const Conf& GetConf() { return DiskCleanup::GetConf(); }


   std::vector< std::tuple< std::string, std::string> >& 
                 GetOlderFilesFromPath(boost::filesystem::path path, const time_t oldestTime) {
      return DiskCleanup::GetOlderFilesFromPath(path,oldestTime);
   }

   void MarkFilesAsRemovedInES(const IdsAndIndexes& relevantRecords, ElasticSearch& es) {
      DiskCleanup::MarkFilesAsRemovedInES(relevantRecords, es);
   }
   size_t BruteForceCleanupOfOldFiles(const boost::filesystem::path& path,
                     const time_t theOldestTime, size_t& additionalSizeRemoved) {
      return DiskCleanup::BruteForceCleanupOfOldFiles( path,theOldestTime,additionalSizeRemoved);
   }
   bool mFailRemoveSearch;
   bool mFailFileSystemInfo;
   int mFileSystemInfoCountdown;
   bool mSucceedRemoveSearch;
   struct statvfs mFleSystemInfo;
   bool mRealFilesSystemAccess;
};
