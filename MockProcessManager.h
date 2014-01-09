#pragma once
#include "ProcessManager.h"

class MockProcessManager : public ProcessManager {
public:

   MockProcessManager(const Conf& conf, const std::string& programName) : ProcessManager(conf, programName), mKillFails(false),
   mExecFails(false), mRealInit(true) {
   }

   virtual ~MockProcessManager() {
   }

   virtual bool Initialize() {
      if (mRealInit) {
         return ProcessManager::Initialize();
      } else {
         return true;
      }
   }
   using ProcessManager::StartDaemon;
   using ProcessManager::StopProcess;

   bool KillPid(pid_t pid) override {
      if (mKillFails) {
         return false;
      }
      return ProcessManager::KillPid(pid);
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
   LR_VIRTUAL pid_t GetParentPid() {
      return ProcessManager::GetParentPid();
   }
   bool mKillFails;
   bool mExecFails;
   std::string mPidDir;
   bool mRealInit;
};

class MockProcessManagerNoInit : public MockProcessManager {
public:

   MockProcessManagerNoInit(const Conf& conf, const std::string& programName) : MockProcessManager(conf, programName) {
      mRealInit = false;
   }

   virtual ~MockProcessManagerNoInit() {
   }

};

class MockProcessManagerNoMotherForker : public MockProcessManager {
public:

   MockProcessManagerNoMotherForker(const Conf& conf, const std::string& programName) : MockProcessManager(conf, programName) {

   }

   pid_t GetParentPid() {
      return 1;
   }

   virtual ~MockProcessManagerNoMotherForker() {
   }

};