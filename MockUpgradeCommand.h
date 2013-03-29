#pragma once

#include "UpgradeCommand.h"
#include <memory>

class MockUpgradeCommand : public UpgradeCommand {
public:
   static Command* Construct(const protoMsg::CommandRequest& request) {
      MockUpgradeCommand* command = new MockUpgradeCommand(request);
      command->SetUploadDir("/tmp/");
      return command;
   }
   virtual protoMsg::CommandReply Execute(const Conf& conf) {
      protoMsg::CommandReply reply;
      reply.set_success(true);
      return std::move(reply);
   }
protected:
   void SetUploadDir(const std::string& dir) {
      mUploadDir = dir;
   }
   explicit MockUpgradeCommand(const protoMsg::CommandRequest& request) : UpgradeCommand(request) {}
   MockUpgradeCommand() {}
   MockUpgradeCommand(const MockUpgradeCommand& ) {}
};
