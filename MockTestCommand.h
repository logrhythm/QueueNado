#pragma once
#include "include/global.h"
#include "ProcessManager.h"
#include "Command.h"
#include "Conf.h"
#include <memory>

class MockTestCommand : public Command {
public:

   virtual ~MockTestCommand() {
   }

   virtual protoMsg::CommandReply Execute(const Conf& conf) {
      protoMsg::CommandReply reply;
      reply.set_success(mSuccess);
      reply.set_result(mResult);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      return reply;
   }

   static std::shared_ptr<Command> Construct(const protoMsg::CommandRequest& request) {
      std::shared_ptr<Command> command(new MockTestCommand(request));
      return command;
   }

   bool mSuccess;
   std::string mResult;
   protoMsg::CommandRequest mRequest;
protected:

   explicit MockTestCommand(const protoMsg::CommandRequest& request) :
   mResult("TestCommand"), mSuccess(true), mRequest(request) {

   }

};

