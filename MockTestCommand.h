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
      unsigned int randSleep = rand() % 10;
      std::this_thread::sleep_for(std::chrono::milliseconds(randSleep));
      return reply;
   }

   static std::shared_ptr<Command> Construct(const protoMsg::CommandRequest& request) {
      std::shared_ptr<Command> command(new MockTestCommand(request));
      return command;
   }
   protoMsg::CommandReply GetResult() {
      return mAsyncResult;
   }
   bool Finished() {
      return mFinished;
   }
   bool mSuccess;
   std::string mResult;
   protoMsg::CommandRequest mRequest;
protected:

   explicit MockTestCommand(const protoMsg::CommandRequest& request) :
   mResult("TestCommand"), mSuccess(true), mRequest(request) {

   }

};

class MockTestCommandAlwaysFails : public MockTestCommand {
public:
   virtual ~MockTestCommandAlwaysFails() {
   }
   static std::shared_ptr<Command> Construct(const protoMsg::CommandRequest& request) {
      std::shared_ptr<Command> command(new MockTestCommandAlwaysFails(request));
      return command;
   }

protected:

   explicit MockTestCommandAlwaysFails(const protoMsg::CommandRequest& request) : MockTestCommand(request) {
      mResult="TestCommandFails";
      mSuccess=false;
      mRequest=request;
   }
};

class MockTestCommandRunsForever : public MockTestCommand {
public:
   virtual ~MockTestCommandRunsForever() {
   }
   static std::shared_ptr<Command> Construct(const protoMsg::CommandRequest& request) {
      std::shared_ptr<Command> command(new MockTestCommandRunsForever(request));
      return command;
   }
   virtual protoMsg::CommandReply Execute(const Conf& conf) {
      protoMsg::CommandReply reply;
      reply.set_success(mSuccess);
      reply.set_result(mResult);
      unsigned int randSleep = 1000000;
      std::this_thread::sleep_for(std::chrono::milliseconds(randSleep));
      return reply;
   }
protected:

   explicit MockTestCommandRunsForever(const protoMsg::CommandRequest& request) : MockTestCommand(request) {

   }
};