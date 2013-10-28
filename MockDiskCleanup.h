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

   size_t RemoveOldestPCapFilesInES(const size_t maxToRemove, const size_t filesPerIteration,
           ElasticSearch& es, size_t& spaceSaved, time_t& oldest) {
      return DiskCleanup::RemoveOldestPCapFilesInES(maxToRemove, filesPerIteration, es, spaceSaved, oldest);
   }

   bool TooMuchPCap(DiskCleanup::StatInfo& stats) LR_OVERRIDE {
      return DiskCleanup::TooMuchPCap(stats);
   }

   LR_VIRTUAL size_t GetFileCountFromES(ElasticSearch& es) {
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

   void RecalculatePCapDiskUsed(StatInfo& stats, ElasticSearch& es) LR_OVERRIDE {
      DiskCleanup::RecalculatePCapDiskUsed(stats, es);
   }

   void CleanupOldPcapFiles(PacketCaptureFilesystemDetails& previous, ElasticSearch& es, SendStats& sendQueue,
           StatInfo& stats) LR_OVERRIDE {
      DiskCleanup::CleanupOldPcapFiles(previous, es, sendQueue, stats);
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

   LR_VIRTUAL void GetPcapStoreUsage(DiskCleanup::StatInfo& stats,
           const DiskUsage::Size size) LR_OVERRIDE {
      if (mRealFilesSystemAccess) {
         DiskCleanup::GetPcapStoreUsage(stats, size);
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
         stats.pcapDiskInGB.Free = disk.DiskFree(size);
         stats.pcapDiskInGB.Total = disk.DiskTotal(size);
         stats.pcapDiskInGB.Used = disk.DiskUsed(size);
      }
   }

   void GetProbeFileSystemInfo(DiskCleanup::StatInfo& stats,
           const DiskUsage::Size size) LR_OVERRIDE {
      if (!mFailFileSystemInfo) {
         if (mRealFilesSystemAccess) {
            DiskCleanup::GetProbeFileSystemInfo(stats, size);
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
            stats.probeDiskInGB.Free = disk.DiskFree(size);
            stats.probeDiskInGB.Total = disk.DiskTotal(size);
            stats.probeDiskInGB.Used = disk.DiskUsed(size);
         }
      }
      if (mFileSystemInfoCountdown-- == 1) {
         stats.probeDiskInGB.Free = stats.probeDiskInGB.Total;
      }
      return;
   }

   void CleanupSearch(PacketCaptureFilesystemDetails& previous,
           ElasticSearch& es, SendStats& sendQueue, DiskCleanup::StatInfo& stats) LR_OVERRIDE {
      DiskCleanup::CleanupSearch(previous, es, sendQueue, stats);
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

   LR_VIRTUAL bool MarkFilesAsRemovedInES(const IdsAndIndexes& relevantRecords, ElasticSearch& es) {
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

   bool WayTooManyFiles(const StatInfo& stats) LR_OVERRIDE {
      return DiskCleanup::WayTooManyFiles(stats);
   }

   size_t IterationTargetToRemove(const StatInfo& stats) LR_OVERRIDE {
      return DiskCleanup::IterationTargetToRemove(stats);
   }

   bool LastIterationAmount(const size_t targetToRemove) {
      return DiskCleanup::LastIterationAmount(targetToRemove);
   }

   size_t CleanupMassiveOvershoot(const size_t targetToRemove, const StatInfo& stats) LR_OVERRIDE {
      return DiskCleanup::CleanupMassiveOvershoot(targetToRemove,stats);
   }

   bool RemoveFile(const std::string& path) {
      if (mFakeRemove) {
         return mRemoveResult;
      }
      return DiskCleanup::RemoveFile(path);
   }

   LR_VIRTUAL int RemoveFiles(const PathAndFileNames& filesToRemove, size_t& spaceSavedInMB, size_t& filesNotFound) LR_OVERRIDE {
      return DiskCleanup::RemoveFiles(filesToRemove, spaceSavedInMB, filesNotFound);
   }

   size_t CalculateNewTotalFiles(const size_t oldTotal,
           const size_t targetRemoved, const size_t failedRemoved) {
      return DiskCleanup::CalculateNewTotalFiles(oldTotal, targetRemoved, failedRemoved);
   }

   bool OptimizeIndexes(const std::set<std::string>& allIndexes,
           const std::set<std::string> excludes, ElasticSearch& es) LR_OVERRIDE {
      DiskCleanup::OptimizeIndexes(allIndexes, excludes, es);
   }

   std::set<std::string> GetIndexesThatAreActive() {
      return DiskCleanup::GetIndexesThatAreActive();
   }

   LR_VIRTUAL void OptimizeThread(ElasticSearch& es) {
      return DiskCleanup::OptimizeThread(es);
   }

   LR_VIRTUAL bool RunOptimize(ElasticSearch& es) {
      return DiskCleanup::RunOptimize(es);
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
using ::testing::Return;
using ::testing::Throw;
using ::testing::DoAll;
using ::testing::SaveArg;

class GMockDiskCleanup : public MockDiskCleanup {
public:

   GMockDiskCleanup(networkMonitor::ConfSlave& conf) : MockDiskCleanup(conf), mFileCount(0), mMarkResult(true) {
   }

   MOCK_METHOD0(IsShutdown, bool());
   MOCK_METHOD3(RemoveFiles, int(const PathAndFileNames& filesToRemove, size_t& spaceSavedInMB, size_t& filesNotFound));
   MOCK_METHOD1(GetFileCountFromES, size_t(ElasticSearch& es));
   MOCK_METHOD2(GetPcapStoreUsage, void(DiskCleanup::StatInfo& pcapDiskInGB, const DiskUsage::Size size));
   MOCK_METHOD2(MarkFilesAsRemovedInES, bool(const IdsAndIndexes& relevantRecords, ElasticSearch& es));
   MOCK_METHOD1(RunOptimize, bool(ElasticSearch& es));

   void DelegateIsShutdownAlwaysTrue() {
      EXPECT_CALL(*this, IsShutdown())
              .WillOnce(Return(true));
   }

   void DelegateGetFileCountFromES(size_t value) {
      mFileCount = value;
      EXPECT_CALL(*this, GetFileCountFromES(_))
              .WillRepeatedly(Return(mFileCount));
   }

   void DelegateRemoveFiles(size_t value) {
      mFailFileCount = value;
      EXPECT_CALL(*this, RemoveFiles(_, _, _))
              .WillRepeatedly(Return(mFailFileCount));
   }

   void DelegateGetPcapStoreUsage() {
      EXPECT_CALL(*this, GetPcapStoreUsage(_, _))
              .WillRepeatedly(Return());
   }

   void GetFileCountFromESThrows() {
      EXPECT_CALL(*this, GetPcapStoreUsage(_, _))
              .WillRepeatedly(Throw(123));
   }

   void DelegateMarkFilesAsRemovedInESExpectNTimes(unsigned int times, bool markResult) {
      mMarkResult = markResult;
      EXPECT_CALL(*this, MarkFilesAsRemovedInES(_, _))
              .Times(times)
              .WillRepeatedly(DoAll(SaveArg<0>(&mRecordsMarked), Return(mMarkResult)));
   }
   size_t mFileCount;
   size_t mFailFileCount;
   bool mMarkResult;
   IdsAndIndexes mRecordsMarked;
};

