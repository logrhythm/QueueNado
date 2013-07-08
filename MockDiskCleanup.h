#pragma once
#include "DiskCleanup.h"

class MockDiskCleanup : public DiskCleanup {
public:

   MockDiskCleanup(networkMonitor::ConfSlave& conf) : DiskCleanup(conf) {
   }

   virtual ~MockDiskCleanup() {
   }

   void RemoveOldestPCapFile() {
      DiskCleanup::RemoveOldestPCapFiles(1);
   }

   void RemoveOldestPCapFiles(const size_t max) {
      DiskCleanup::RemoveOldestPCapFiles(1);
   }

   bool TooMuchPCap(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      return DiskCleanup::TooMuchPCap(aDiskUsed, aTotalFiles);
   }

   void RecalculateDiskUsed(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      DiskCleanup::RecalculateDiskUsed(aDiskUsed, aTotalFiles);
   }

   void CleanupOldPcapFiles(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      DiskCleanup::CleanupOldPcapFiles(aDiskUsed, aTotalFiles);
   }
};