#pragma once
#include "ProcessManager.h"
#include "Conf.h"

class MockProcessManagerCommand : public ProcessManager {
public:

   MockProcessManagerCommand(const Conf& conf) : ProcessManager(conf) {
      mSuccess = true;
      mReturnCode = 0;
      mResult = "Success";
      mInit = true;
   }

   bool Initialize() {
      return mInit;
   }

   const std::string& getRunCommand() {
      return mRunCommand;
   }

   const std::string& getRunArgs() {
      return mRunArgs;
   }

   virtual void setInit(bool init) {
      mInit = init;
   }

   virtual void SetReturnCode(const int returnCode) {
      mReturnCode = returnCode;

   }

   virtual void SetSuccess(const bool success) {
      mSuccess = success;

   }

   virtual void SetResult(const std::string result) {
      mResult = result;
   }

   protoMsg::ProcessReply RunProcess(const std::string& execPath, const std::string& args) {
      mRunCommand.clear();
      mRunCommand = execPath;
      mRunArgs.clear();
      mRunArgs = args;
      protoMsg::ProcessReply reply;
      reply.set_success(mSuccess);
      reply.set_returncode(mReturnCode);
      reply.set_result(mResult);
      return reply;
   }

   virtual ~MockProcessManagerCommand() {
   }


   int mReturnCode;
   bool mInit;
   bool mSuccess;
   std::string mResult;
   std::string mRunCommand;
   std::string mRunArgs;
   private:
};
