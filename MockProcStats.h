#pragma once

#include "ProcStats.h"
#include "include/global.h"
#include "gmock/gmock.h"
#include "MemoryInfo.h"

class GMockMemoryInfo : public MemoryInfo{
   public:
      MOCK_METHOD0(ReadTotalMemMBOnce, size_t());
};



class MockProcStats : public ProcStats {
public:
   bool mPseudoTask; 
   bool mPseudoThreadPid;
   pid_t mPseudoPid;
   std::string mPseudoProcTaskFileName;
   GMockMemoryInfo mGMockMemoryInfo;
   size_t mPseudoTotalMemMB;
   
   bool mUsePseudoCpuJiffies;
   CpuJiffies mPseudoCpuJiffies;
   
public:
   MockProcStats(): mPseudoTask(false), mPseudoThreadPid(false), 
           mUsePseudoCpuJiffies(false) {}
   
   MockProcStats(bool pseudoTask, bool pseudoThreadPid, size_t pseudoTotalMem, bool pseudoJiffies)
   : mPseudoTask(pseudoTask), mPseudoThreadPid(pseudoThreadPid), 
           mUsePseudoCpuJiffies(pseudoJiffies) {}

   ~MockProcStats() {}
   bool UpdateMemStats() {
      return ProcStats::UpdateMemStats();
   }
   
   MemoryInfo& GetMemoryInfo() LR_OVERRIDE{
   return mGMockMemoryInfo;
}
   
   
   CpuJiffies UpdateSystemCPU() {
      if (mUsePseudoCpuJiffies) {
         return mPseudoCpuJiffies;
      }
      return ProcStats::UpdateSystemCPU();
   }
   
   bool UpdateCpuStats() {
       return ProcStats::UpdateCpuStats();
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
};
