#pragma once
#include "include/global.h"
#include "DpiMsgLRPool.h"

#ifdef LR_DEBUG

class MockDpiMsgLRPool : public DpiMsgLRPool {
public:

   MockDpiMsgLRPool() {
   }

   virtual ~MockDpiMsgLRPool() {
   }

   bool DpiMsgTooBig(networkMonitor::DpiMsgLR* reusedMsg, int limit) override {
      return DpiMsgLRPool::DpiMsgTooBig(reusedMsg, limit);
   }

   bool ReportSize(const pthread_t thread, const DpiMsgLRPool::FreePoolOfMessages& freePool, const DpiMsgLRPool::UsedPoolOfMessages& usedPool) {
      return DpiMsgLRPool::ReportSize(thread, freePool, usedPool);
   }

   bool OverGivenThreshold(const int fSize, const int uSize, const double percent, const int minAllowedFree, const int maxAllowedFree) {
      return DpiMsgLRPool::OverGivenThreshold(fSize, uSize, percent, minAllowedFree, maxAllowedFree);
   }

   SendStats* GetStatsSender(pthread_t thread) {
      return DpiMsgLRPool::GetStatsSender(thread);
   }

   time_t GetStatsTimer(pthread_t thread) {
      return DpiMsgLRPool::GetStatsTimer(thread);
   }

   void SetStatsTimer(pthread_t thread, time_t time) {
      return DpiMsgLRPool::SetStatsTimer(thread, time);
   }
};
#endif
