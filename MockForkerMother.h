#pragma once
#include "ForkerMother.h"
#include <map>

class MockForkerMother : public ForkerMother {
public:

   MockForkerMother() : ForkerMother() {
   }

   virtual ~MockForkerMother() {
   }

   LR_VIRTUAL pid_t Fork() LR_OVERRIDE {
      return mForkPid;
   }
   LR_VIRTUAL void Exit(int status) LR_OVERRIDE {
      mExitStatus = status;
   }
   LR_VIRTUAL pid_t WaitPid(pid_t pid, int *stat_loc, int options) LR_OVERRIDE {
      return mWaitPidResult[pid];
   }
   LR_VIRTUAL int Kill (pid_t pid, int sig) LR_OVERRIDE {
      return mKillResult[pid];
   }
   pid_t mForkPid;
   int mExitStatus;
   std::map<pid_t,pid_t> mWaitPidResult;
   std::map<pid_t,int> mKillResult;
};

