#pragma once

#include "ProcStats.h"
#include "include/global.h"

class MockProcStats : public ProcStats {
public:
   MockProcStats() {}
   ~MockProcStats() {}
   bool UpdateMemStats() {
      return ProcStats::UpdateMemStats();
   }
   std::vector<CpuJiffies> UpdateSystemCPU() {
      return ProcStats::UpdateSystemCPU();
   }
   void SetMemFile(const std::string& memFile) {
      mProcMeminfoName = memFile;
   }
   void SetStatFile(const std::string& statFile) {
      mProcStatName = statFile;
   }
private:
         
};