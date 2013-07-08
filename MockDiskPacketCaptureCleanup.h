#pragma once
#include "DiskPacketCaptureCleanup.h"

class MockDiskPacketCaptureCleanup : public DiskPacketCaptureCleanup {
public:

   MockDiskPacketCaptureCleanup(Conf& conf) : DiskPacketCaptureCleanup(conf) {
   }

   virtual ~MockDiskPacketCaptureCleanup() {
   }

   void RemoveOldestPCapFile() {
      DiskPacketCaptureCleanup::RemoveOldestPCapFiles(1);
   }

   void RemoveOldestPCapFiles(const size_t max) {
      DiskPacketCaptureCleanup::RemoveOldestPCapFiles(1);
   }

   bool TooMuchPCap(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      return DiskPacketCaptureCleanup::TooMuchPCap(aDiskUsed, aTotalFiles);
   }

   void RecalculateDiskUsed(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      DiskPacketCaptureCleanup::RecalculateDiskUsed(aDiskUsed, aTotalFiles);
   }

   void CleanupOldPcapFiles(std::atomic<size_t>& aDiskUsed, std::atomic<size_t>& aTotalFiles) {
      DiskPacketCaptureCleanup::CleanupOldPcapFiles(aDiskUsed, aTotalFiles);
   }
};