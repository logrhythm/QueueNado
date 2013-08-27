#pragma once

#include "ProcStats.h"
#include "include/global.h"

class MockProcStats : public ProcStats {
   bool mPseudoTask; 
   bool mPseudoThreadPid;
   pid_t mPseudoPid;
   std::string mPseudoProcTaskFileName;
public:
   MockProcStats(): mPseudoTask(false), mPseudoThreadPid(false) {}
   ~MockProcStats() {}
   bool UpdateMemStats() {
      return ProcStats::UpdateMemStats();
   }
   CpuJiffies UpdateSystemCPU() {
      return ProcStats::UpdateSystemCPU();
   }
   void SetMemFile(const std::string& memFile) {
      mProcMeminfoName = memFile;
   }
   void SetStatFile(const std::string& statFile) {
      mProcStatName = statFile;
   }
   
   void SetTaskPseudoFile(const std::string& taskStatFile) {
      if (!taskStatFile.empty()) {
         mPseudoTask = true;
         mPseudoProcTaskFileName = taskStatFile;
      } else {
         mPseudoTask = false;
      }
   }
   
   ThreadJiffies GetCurrentThreadJiffies(const pid_t threadId) {
      if (mPseudoTask) {
         return ThreadJiffies(mPseudoProcTaskFileName);
      } 
      return ProcStats::GetCurrentThreadJiffies(threadId);
   }
   
   void SetPseudoThreadPid(pid_t pid) {
      mPseudoThreadPid = true;
      mPseudoPid = pid;
   }
   
   pid_t GetThreadID()  {
      if(mPseudoThreadPid) {
         return mPseudoPid;
      }
      
      return ProcStats::GetThreadID();
   }
   
     
private:
         
};