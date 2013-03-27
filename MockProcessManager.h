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

   int RunExecV(const char* command, char** arguments) {
      if (mExecFails) {
         return -1;
      }
      return ProcessManager::RunExecV(command,arguments);
   }
   bool mKillFails;
   bool mExecFails;
};