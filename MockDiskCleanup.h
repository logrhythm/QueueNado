#pragma once
#include "DiskCleanup.h"
#include "MockDiskUsage.h"
#include "MockConf.h"
#include <sys/statvfs.h>
#include <MockDiskUsage.h>
#include "BoolReturns.h"
#include "gmock/gmock.h"
#include "MockPcapDiskUsage.h"
#include "include/global.h"
#include <vector>
extern std::string gProgramName;

class MockDiskCleanup : public DiskCleanup {
public:

   MockDiskCleanup(networkMonitor::ConfSlave& conf, const std::string& programName) : DiskCleanup(conf,programName), mFailRemoveSearch(false),
   mFailFileSystemInfo(false), mFileSystemInfoCountdown(0), mSucceedRemoveSearch(false),
   mRealFilesSystemAccess(false), mFakeRemove(false), mRemoveResult(true),mFakeIsShutdown(false),
           mIsShutdownResult(false), mDoPseudoGetUpdatedDiskInfo(false), mUseMockConf(false), 
           mMockPcapDiskUsage(DiskCleanup::GetConf().GetPcapCaptureLocations(), programName) {
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
           ElasticSearch& es, size_t& spaceSaved, time_t& oldest, StatInfo& stats) {
      return DiskCleanup::RemoveOldestPCapFilesInES(maxToRemove, filesPerIteration, es, spaceSaved, oldest, stats);
   }

   bool TooMuchPCap(DiskCleanup::StatInfo& stats) LR_OVERRIDE {
      return DiskCleanup::TooMuchPCap(stats);
   }

   LR_VIRTUAL bool GetFileCountFromES(ElasticSearch& es, size_t& totalFiles) {
      auto pcapLocations = mConf.GetPcapCaptureLocations();
      totalFiles = 0;
      for (const auto& path : pcapLocations) {

         for (boost::filesystem::directory_iterator it(path);
                 it != boost::filesystem::directory_iterator(); it++) {
            if (IsShutdown()) {
               return false; //caught shutdown;
            }
            totalFiles++;
         }
      }
      return true;
   }

   void RecalculatePCapDiskUsed(StatInfo& stats, ElasticSearch& es) LR_OVERRIDE {
      DiskCleanup::RecalculatePCapDiskUsed(stats, es);
   }

   void CleanupOldPcapFiles(ElasticSearch& es, StatInfo& stats) LR_OVERRIDE {
      DiskCleanup::CleanupOldPcapFiles(es, stats);
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
           const MemorySize size) LR_OVERRIDE {
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
         MockDiskUsage disk(mockStatvs,gProgramName);

         disk.Update();
         stats.pcapDiskInGB.Free = disk.DiskFree(size);
         stats.pcapDiskInGB.Total = disk.DiskTotal(size);
         stats.pcapDiskInGB.Used = disk.DiskUsed(size);
      }
   }

   void GetProbeFileSystemInfo(DiskCleanup::StatInfo& stats,
           const MemorySize size) LR_OVERRIDE {
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
            MockDiskUsage disk(mockStatvs,gProgramName);

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

   void CleanupSearch(ElasticSearch& es, DiskCleanup::StatInfo& stats) LR_OVERRIDE {
      DiskCleanup::CleanupSearch(es, stats);
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
   
   PcapDiskUsage& GetPcapUsageInstance() LR_OVERRIDE {
      mMockPcapDiskUsage.mLocations = GetConf().GetPcapCaptureLocations();
      return mMockPcapDiskUsage;
   }

   size_t RemoveOlderFilesFromPath(boost::filesystem::path path, const time_t oldestTime, size_t& spaceSaved) {
      return DiskCleanup::RemoveOlderFilesFromPath(path, oldestTime, spaceSaved);
   }

   LR_VIRTUAL bool MarkFilesAsRemovedInES(const IdsAndIndexes& relevantRecords, const networkMonitor::DpiMsgLR& updateMsg, ElasticSearch& es) {
      return DiskCleanup::MarkFilesAsRemovedInES(relevantRecords, updateMsg, es);
   }

   size_t BruteForceCleanupOfOldFiles(const boost::filesystem::path& path,
           const time_t theOldestTime, size_t& additionalSizeRemoved) {
      return DiskCleanup::BruteForceCleanupOfOldFiles(path, theOldestTime, additionalSizeRemoved);
   }

   bool TimeForBruteForceCleanup(const std::time_t& lastForcedDiskClean) LR_OVERRIDE {
      return DiskCleanup::TimeForBruteForceCleanup(lastForcedDiskClean);
   }

   void SetLastForcedClean(const time_t newTime,std::time_t& lastForcedDiskClean) LR_OVERRIDE{
      DiskCleanup::SetLastForcedClean(newTime,lastForcedDiskClean);
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

      
   void SendAllStats(ElasticSearch& es,StatInfo& stats) {
       
       // Send all stats either to a Mock sendQueue or the real one
       DiskCleanup::SendAllStats(es,stats);
   }
      
   
   bool GetUpdatedDiskInfo(ElasticSearch& es, StatInfo& stats) {
       if (mDoPseudoGetUpdatedDiskInfo) {
           stats.pcapFilesystemDetails = mPseudoGetUpdatedDiskInfo;
           return true;
       }
       
       return DiskCleanup::GetUpdatedDiskInfo(es, stats);
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
   void RemoveDuplicateUpdatesFromLastUpdate(PathAndFileNames& esFilesToRemove) {
      DiskCleanup::RemoveDuplicateUpdatesFromLastUpdate(esFilesToRemove);
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
   bool mDoPseudoGetUpdatedDiskInfo;
   DiskCleanup::PacketCaptureFilesystemDetails mPseudoGetUpdatedDiskInfo;
   
   bool mUseMockConf;
   MockConf mMockedConf;
   MockPcapDiskUsage mMockPcapDiskUsage;
};
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Throw;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::SetArgReferee;
class GMockDiskCleanup : public MockDiskCleanup {
public:

   GMockDiskCleanup(networkMonitor::ConfSlave& conf, const std::string programName) : MockDiskCleanup(conf,programName), mFileCount(0), mMarkResult(true),
   mFileCountSuccess(true) {
   }

   MOCK_METHOD0(IsShutdown, bool());
   MOCK_METHOD3(RemoveFiles, int(const PathAndFileNames& filesToRemove, size_t& spaceSavedInMB, size_t& filesNotFound));
   MOCK_METHOD2(GetFileCountFromES, bool(ElasticSearch& es,size_t& count));
   MOCK_METHOD2(GetPcapStoreUsage, void(DiskCleanup::StatInfo& pcapDiskInGB, const MemorySize size));
   MOCK_METHOD3(MarkFilesAsRemovedInES, bool(const IdsAndIndexes& relevantRecords, const networkMonitor::DpiMsgLR& updateMsg,ElasticSearch& es));
   MOCK_METHOD1(RunOptimize, bool(ElasticSearch& es));

   void DelegateIsShutdownAlwaysTrue() {
      EXPECT_CALL(*this, IsShutdown())
              .WillOnce(Return(true));
   }

   void DelegateGetFileCountFromES(size_t value,bool success) {
      mFileCount = value;
      mFileCountSuccess = success;
      EXPECT_CALL(*this, GetFileCountFromES(_,_))
              .WillRepeatedly(DoAll(SetArgReferee<1>(mFileCount), Return(success)));
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
      EXPECT_CALL(*this, MarkFilesAsRemovedInES(_,_,_))
              .Times(times)
              .WillRepeatedly(DoAll(SaveArg<0>(&mRecordsMarked), Return(mMarkResult)));
   }
   size_t mFileCount;
   size_t mFailFileCount;
   bool mMarkResult;
   IdsAndIndexes mRecordsMarked;
   networkMonitor::DpiMsgLR updateMsg;
   bool mFileCountSuccess;
};

