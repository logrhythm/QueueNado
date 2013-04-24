#pragma once

#include "ProcStats.h"

class MockProcStats : public ProcStats {
public:
   MockProcStats() {}
   ~MockProcStats() {}
   bool UpdateMemStats() {
      return ProcStats::UpdateMemStats();
   }
   bool UpdateSystemCPU() {
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