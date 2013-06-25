#pragma once
#include "ProcessManager.h"

class MockProcessManager : public ProcessManager {
public:

   MockProcessManager(const Conf& conf) : ProcessManager(conf), mKillFails(false),
   mExecFails(false) {
   }

   virtual ~MockProcessManager() {
   }

   using ProcessManager::StartDaemon;
   using ProcessManager::StopProcess;

   bool KillPid(pid_t pid) override {
      if (mKillFails) {
         return false;
      }
      return ProcessManager::KillPid(pid);
   }
   
   //no override keyword because it's LRVIRTUAL
   int RunExecVE(const char* command, char** arguments, char** environment) {
      if (mExecFails) {
         return -1;
      }
      return ProcessManager::RunExecVE(command, arguments, environment);
   }

   virtual bool CheckPidExists(const int pid) override {
      return ProcessManager::CheckPidExists(pid);
   }

   virtual int ReclaimPid(const protoMsg::ProcessRequest& request, const int pid) override {
      return ProcessManager::ReclaimPid(request, pid);
   }

   virtual int GetPidFromFiles(const protoMsg::ProcessRequest& request) override {
      return ProcessManager::GetPidFromFiles(request);
   }

   virtual bool WritePid(const int pid) override {
      return ProcessManager::WritePid(pid);
   }

   virtual bool DeletePid(const int pid) override {
      return ProcessManager::DeletePid(pid);
   }

   virtual void SetPidDir(const std::string pidDir) override {
      ProcessManager::SetPidDir(pidDir);
   }

   bool mKillFails;
   bool mExecFails;
   std::string mPidDir;
};