#pragma once
#include "ProcessManager.h"

class MockProcessManager : public ProcessManager {
public:      
   MockProcessManager(const Conf& conf) : ProcessManager(conf), mKillFails(false) {}
   virtual ~MockProcessManager() {}
   bool KillPid(pid_t pid) {
      if (mKillFails) {
         return false;
      }
      return ProcessManager::KillPid(pid);
   }
   bool mKillFails;
};