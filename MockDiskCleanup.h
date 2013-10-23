#pragma once
#include "DiskCleanup.h"
#include "MockDiskUsage.h"
#include "MockConf.h"
#include <sys/statvfs.h>
#include <MockDiskUsage.h>
#include "BoolReturns.h"
#include "gmock/gmock.h"

class MockDiskCleanup : public DiskCleanup {
public:

   MockDiskCleanup(networkMonitor::ConfSlave& conf) : DiskCleanup(conf), mFailRemoveSearch(false),
   mFailFileSystemInfo(false), mFileSystemInfoCountdown(0), mSucceedRemoveSearch(false),
   mRealFilesSystemAccess(false), mFakeRemove(false), mRemoveResult(true), mFakeIsShutdown(false),
   mIsShutdownResult(false), mUseMockConf(false) {
      mFleSystemInfo.f_bfree = 1;
      mFleSystemInfo.f_frsize = 1;
      mFleSystemInfo.f_blocks = 1;
      mFleSystemInfo.f_frsize = 1;
   }

   virtual ~MockDiskCleanup() {
   }

   virtual bool IsShutdown() {
      if (mFakeIsShutdown) {
         return mIsShutdownResult;
      }
      return false;
   }

   size_t RemoveOldestPCapFilesInES(const size_t maxToRemove, ElasticSearch& es, size_t& spaceSaved, time_t& oldest) {
      return DiskCleanup::RemoveOldestPCapFilesInES(1, es, spaceSaved, oldest);
   }

   bool TooMuchPCap(size_t& aDiskUsed, size_t& aTotalFiles) {
      return DiskCleanup::TooMuchPCap(aDiskUsed, aTotalFiles);
   }

   size_t GetFileCountFromES(ElasticSearch& es) {
      auto pcapLocations = mConf.GetPcapCaptureLocations();
      size_t totalFiles{0};      
      for (const auto& path : pcapLocations) {

         for (boost::filesystem::directory_iterator it(path);
                 it != boost::filesystem::directory_iterator(); it++) {
            if (IsShutdown()) {
               return 0; //caught shutdown;
            }
            totalFiles++;
         }
      }
      return totalFiles;
   }

   void RecalculatePCapDiskUsed(size_t& aDiskUsed, size_t& aTotalFiles, DiskSpace& pcapDiskInGB, ElasticSearch& es) {
      DiskCleanup::RecalculatePCapDiskUsed(aDiskUsed, aTotalFiles, pcapDiskInGB, es);
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
      } else {
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
            DiskCleanup::GetProbeFileSystemInfo(probeDiskInGB, size);
         } else {
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

   Conf& GetConf() LR_OVERRIDE {
      if (false == mUseMockConf) {
         return DiskCleanup::GetConf();
      }
      return mMockedConf;
   }

   std::vector< std::tuple< std::string, std::string> >&
   GetOlderFilesFromPath(boost::filesystem::path path, const time_t oldestTime) {
      return DiskCleanup::GetOlderFilesFromPath(path, oldestTime);
   }

   bool MarkFilesAsRemovedInES(const IdsAndIndexes& relevantRecords, ElasticSearch& es) {
      return DiskCleanup::MarkFilesAsRemovedInES(relevantRecords, es);
   }

   size_t BruteForceCleanupOfOldFiles(const boost::filesystem::path& path,
           const time_t theOldestTime, size_t& additionalSizeRemoved) {
      return DiskCleanup::BruteForceCleanupOfOldFiles(path, theOldestTime, additionalSizeRemoved);
   }

   bool TimeForBruteForceCleanup() {
      return DiskCleanup::TimeForBruteForceCleanup();
   }

   void SetLastForcedClean(const time_t newTime) {
      DiskCleanup::SetLastForcedClean(newTime);
   }

   bool WayTooManyFiles(const size_t aTotalFiles) {
      return DiskCleanup::WayTooManyFiles(aTotalFiles);
   }

   size_t IterationTargetToRemove(const size_t aTotalFiles) {
      return DiskCleanup::IterationTargetToRemove(aTotalFiles);
   }

   bool LastIterationAmount(const size_t targetToRemove) {
      return DiskCleanup::LastIterationAmount(targetToRemove);
   }

   size_t CleanupMassiveOvershoot(const size_t targetToRemove, const size_t aTotalFiles) {
      return DiskCleanup::CleanupMassiveOvershoot(targetToRemove, aTotalFiles);
   }

   bool RemoveFile(const std::string& path) {
      if (mFakeRemove) {
         return mRemoveResult;
      }
      return DiskCleanup::RemoveFile(path);
   }

   int RemoveFiles(const PathAndFileNames& filesToRemove, size_t& spaceSavedInMB, size_t& filesNotFound) {
      return DiskCleanup::RemoveFiles(filesToRemove, spaceSavedInMB, filesNotFound);
   }

   size_t CalculateNewTotalFiles(const size_t oldTotal,
           const size_t targetRemoved, const size_t failedRemoved) {
      return DiskCleanup::CalculateNewTotalFiles(oldTotal, targetRemoved, failedRemoved);
   }

   void OptimizeIndexes(const std::set<std::string>& allIndexes,
           const std::set<std::string> excludes, ElasticSearch& es) {
      DiskCleanup::OptimizeIndexes(allIndexes, excludes, es);
   }

   std::set<std::string> GetIndexesThatAreActive() {
      return DiskCleanup::GetIndexesThatAreActive();
   }

   LR_VIRTUAL void OptimizeThread(ElasticSearch& es) {
      return DiskCleanup::OptimizeThread(es);
   }
   bool mFailRemoveSearch;
   bool mFailFileSystemInfo;
   int mFileSystemInfoCountdown;
   bool mSucceedRemoveSearch;
   struct statvfs mFleSystemInfo;
   bool mRealFilesSystemAccess;
   bool mFakeRemove;
   bool mRemoveResult;
   bool mFakeIsShutdown;
   bool mIsShutdownResult;
   bool mUseMockConf;
   MockConf mMockedConf;
};
using ::testing::_;
using ::testing::Invoke;

class GMockDiskCleanup : public MockDiskCleanup {
public:

   GMockDiskCleanup(networkMonitor::ConfSlave& conf) : MockDiskCleanup(conf) {
   }

   MOCK_METHOD0(IsShutdown, bool());

   void DelegateIsShutdownAlwaysTrue() {
      ON_CALL(*this, IsShutdown())
              .WillByDefault(Invoke(&returnBools, &BoolReturns::ReturnTrue));
   }

   BoolReturns returnBools;
};

