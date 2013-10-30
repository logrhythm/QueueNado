#pragma once

#include "ProcStats.h"
#include "include/global.h"

class MockProcStats : public ProcStats {
public:
   bool mPseudoTask; 
   bool mPseudoThreadPid;
   pid_t mPseudoPid;
   size_t mPseudoTotalMemMB;
   std::string mPseudoProcTaskFileName;
   
   bool mUsePseudoCpuJiffies;
   CpuJiffies mPseudoCpuJiffies;
   
public:
   MockProcStats(): mPseudoTask(false), mPseudoThreadPid(false), mPseudoTotalMemMB(0), mUsePseudoCpuJiffies(false) {}
   MockProcStats(bool pseudoTask, bool pseudoThreadPid, size_t pseudoTotalMem, bool pseudoJiffies): mPseudoTask(pseudoTask), mPseudoThreadPid(pseudoThreadPid), mPseudoTotalMemMB(pseudoTotalMem), mUsePseudoCpuJiffies(pseudoJiffies) {}

   ~MockProcStats() {}
   bool UpdateMemStats() {
      return ProcStats::UpdateMemStats();
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

  void SetPseudoSysConfTotalMemoryMB (size_t pseudoTotalMB) {
     mPseudoTotalMemMB = pseudoTotalMB;
   }
   size_t ReadTotalMemMBOnce() LR_OVERRIDE {
      if(0 != mPseudoTotalMemMB) {
         return mPseudoTotalMemMB;
      }
      return ProcStats::ReadTotalMemMBOnce();
   }         
};
