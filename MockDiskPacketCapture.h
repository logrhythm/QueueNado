#pragma once

#include "DiskPacketCapture.h"
#include <string>

class MockDiskPacketCapture : public DiskPacketCapture {
public:

   MockDiskPacketCapture(Conf& conf) : DiskPacketCapture(conf) {
   }

   virtual ~MockDiskPacketCapture() {
   }
   bool Initialize(const bool diskCleanThread = false) {
      return DiskPacketCapture::Initialize(diskCleanThread);
   }
   void GetRunningPackets(const std::string& uuid, std::pair<InMemoryPacketBuffer*, size_t>*& sessionInfo) {
      DiskPacketCapture::GetRunningPackets(uuid,sessionInfo);
   }


   void RemoveFromRunningPackets(const pthread_t tid,const std::string& uuid) {
      DiskPacketCapture::RemoveFromRunningPackets(tid, uuid);
   }

    std::string BuildFilename( const std::string& uuid) {
      return DiskPacketCapture::BuildFilename( uuid);
   }

   std::string BuildFilenameWithPath( const std::string& uuid) {
      return DiskPacketCapture::BuildFilenameWithPath( uuid);
   }

   int NewTotalMemory(const size_t memoryAddedIfSaved) {
      return DiskPacketCapture::NewTotalMemory(memoryAddedIfSaved);
   }

   int CurrentMemoryForFlow(const std::string& uuid) {
      return DiskPacketCapture::CurrentMemoryForFlow(uuid);
   }
 
};

