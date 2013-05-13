#pragma once

#include "DiskPacketCapture.h"
#include <string>

class MockDiskPacketCapture : public DiskPacketCapture {
public:

   MockDiskPacketCapture(Conf& conf) : DiskPacketCapture(conf) {
   }

   virtual ~MockDiskPacketCapture() {
   }

   void GetRunningPackets(const std::string& uuid, std::pair<InMemoryPacketBuffer*, size_t>*& sessionInfo) {
      DiskPacketCapture::GetRunningPackets(uuid,sessionInfo);
   }

   void RemoveOldestPCapFile() {
      DiskPacketCapture::RemoveOldestPCapFile();
   }

   void RemoveFromRunningPackets(const std::string& uuid) {
      DiskPacketCapture::RemoveFromRunningPackets(uuid);
   }

   bool TooMuchPCap() {
      return DiskPacketCapture::TooMuchPCap();
   }

   std::string BuildFilename( const std::string& uuid,
           const std::string& appName, const std::time_t time) {
      return DiskPacketCapture::BuildFilename( uuid, appName, time);
   }

   int NewTotalMemory(const size_t memoryAddedIfSaved) {
      return DiskPacketCapture::NewTotalMemory(memoryAddedIfSaved);
   }

   int CurrentMemoryForFlow(const std::string& uuid) {
      return DiskPacketCapture::CurrentMemoryForFlow(uuid);
   }
   void RecalculateDiskUsed() {
      DiskPacketCapture::RecalculateDiskUsed();
   }
};

