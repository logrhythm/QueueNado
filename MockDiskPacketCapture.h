#pragma once

#include "DiskPacketCapture.h"
#include <string>

class MockDiskPacketCapture : public DiskPacketCapture {
public:

   MockDiskPacketCapture() : DiskPacketCapture() {
   }

   virtual ~MockDiskPacketCapture() {
   }

   void GetRunningPackets(const std::string& uuid, std::pair<InMemoryPacketBuffer*, size_t>*& sessionInfo) {
      DiskPacketCapture::GetRunningPackets(uuid,sessionInfo);
   }

   void RemoveOldestPCapFile(Conf& conf) {
      DiskPacketCapture::RemoveOldestPCapFile(conf);
   }

   void RemoveFromRunningPackets(const std::string& uuid) {
      DiskPacketCapture::RemoveFromRunningPackets(uuid);
   }

   bool TooMuchPCap(Conf& conf) {
      return DiskPacketCapture::TooMuchPCap(conf);
   }

   std::string BuildFilename(Conf& conf, const std::string& uuid,
           const std::string& appName, const std::time_t time) {
      return DiskPacketCapture::BuildFilename(conf, uuid, appName, time);
   }

   int NewTotalMemory(const size_t memoryAddedIfSaved) {
      return DiskPacketCapture::NewTotalMemory(memoryAddedIfSaved);
   }

   int CurrentMemoryForFlow(const std::string& uuid) {
      return DiskPacketCapture::CurrentMemoryForFlow(uuid);
   }
   void RecalculateDiskUsed(Conf& conf) {
      DiskPacketCapture::RecalculateDiskUsed(conf);
   }
};

