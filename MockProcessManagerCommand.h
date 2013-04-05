#pragma once
#include "ProcessManager.h"
#include "Conf.h"

class MockProcessManagerCommand : public ProcessManager {
public:

   MockProcessManagerCommand(const Conf& conf) : ProcessManager(conf) {
      mSuccess=false;
      mReturnCode=-1;
      mResult="Fail";
   }
   
   bool Initialize() {
      return true;
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

   protoMsg::ProcessReply RunProcess(const std::string, const std::string) {
      protoMsg::ProcessReply reply;
      reply.set_success(mSuccess);
      reply.set_returncode(mReturnCode);
      reply.set_result(mResult);

   }

   virtual ~MockProcessManagerCommand() {
   }
   
   private:
      int mReturnCode;
      bool mSuccess;
      std::string mResult;
};