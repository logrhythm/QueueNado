#pragma once

#include "DiskPacketCapture.h"
#include <string>

class MockDiskPacketCapture : public DiskPacketCapture {
public:

   MockDiskPacketCapture(Conf& conf) : DiskPacketCapture(conf), mPassFlush(false), mFailFlush(false), mSkipWrite(false), mTrackWrites(true) {
   }

   virtual ~MockDiskPacketCapture() {
   }

   bool InitializeStatsSender(SendStats& sendQueue) {
      return false;
   }

   bool Initialize() {
      return DiskPacketCapture::Initialize();
   }

   void GetRunningPackets(const std::string& uuid, SessionInfo*& sessionInfo) {
      DiskPacketCapture::GetRunningPackets(uuid, sessionInfo);
   }

   bool WriteSavedSessionToDisk(networkMonitor::DpiMsgLR* dpiMsg) {
      if (mTrackWrites) {
         mFilesWritten.push_back(dpiMsg->session_id());
      }
      return DiskPacketCapture::WriteSavedSessionToDisk(dpiMsg);
   }
   bool UnprotectedWriteSavedSessionToDisk(networkMonitor::DpiMsgLR* dpiMsg) {
      if (mSkipWrite) {

         return true;
      }
      return DiskPacketCapture::UnprotectedWriteSavedSessionToDisk(dpiMsg);
   }
   void RemoveFromRunningPackets(const pthread_t tid, const std::string& uuid) {
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
   void ClearFromBigSessions(networkMonitor::DpiMsgLR* dpiMsg) {
      return DiskPacketCapture::ClearFromBigSessions(dpiMsg);
   }
   BigFlows* GetBigFlows() {
      return DiskPacketCapture::GetBigFlows();
   }
   bool mFailFlush;
   bool mPassFlush;
   bool mSkipWrite;
   bool mTrackWrites;
   std::vector<std::string> mFilesWritten;
};

