#pragma once
#include "ProcessManager.h"

class MockProcessManager : public ProcessManager {
public:

   MockProcessManager(const Conf& conf) : ProcessManager(conf), mKillFails(false),
                      mExecFails(false){
   }

   virtual ~MockProcessManager() {
   }

   bool KillPid(pid_t pid) {
      if (mKillFails) {
         return false;
      }
      return ProcessManager::KillPid(pid);
   }

   int RunExecVE(const char* command, char** arguments, char** environment) {
      if (mExecFails) {
         return -1;
      }
      return ProcessManager::RunExecVE(command,arguments,environment);
   }
   bool mKillFails;
   bool mExecFails;
};