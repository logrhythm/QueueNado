#pragma once

#include "DiskPacketCapture.h"
#include <string>

class MockDiskPacketCapture : public DiskPacketCapture {
public:

   MockDiskPacketCapture(Conf& conf) : DiskPacketCapture(conf), mPassFlush(false), mFailFlush(false) {
   }

   virtual ~MockDiskPacketCapture() {
   }
   bool InitializeStatsSender(SendStats& sendQueue) {
      return false;
   }
   bool Initialize(const bool diskCleanThread = false) {
      return DiskPacketCapture::Initialize(diskCleanThread);
   }
   void GetRunningPackets(const std::string& uuid, SessionInfo*& sessionInfo) {
      DiskPacketCapture::GetRunningPackets(uuid,sessionInfo);
   }
   
   bool WriteSavedSessionToDisk(networkMonitor::DpiMsgLR* dpiMsg) {
      mFilesWritten.push_back(dpiMsg->sessionid());
      return DiskPacketCapture::WriteSavedSessionToDisk(dpiMsg);
   }

   void RemoveFromRunningPackets(const pthread_t tid,const std::string& uuid) {
      DiskPacketCapture::RemoveFromRunningPackets(tid, uuid);
   }

   int NewTotalMemory(const size_t memoryAddedIfSaved) {
      return DiskPacketCapture::NewTotalMemory(memoryAddedIfSaved);
   }

   int CurrentMemoryForFlow(const std::string& uuid) {
      return DiskPacketCapture::CurrentMemoryForFlow(uuid);
   }
   
   int CurrentDiskForFlow(const std::string & sessionId) {
      return DiskPacketCapture::CurrentDiskForFlow(sessionId);
   }
   
   bool FlushABigSession() {
      if (mFailFlush) {
         return false;
      }
      if (mPassFlush) {
         return true;
      }
      return DiskPacketCapture::FlushABigSession();
   }
   bool mFailFlush;
   bool mPassFlush;
   std::vector<std::string> mFilesWritten;
};

