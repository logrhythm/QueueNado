#pragma once

#include "UpgradeCommand.h"
#include <memory>
#include "MockProcessClientCommand.h"

class MockUpgradeCommand : public UpgradeCommand {
public:
   static std::shared_ptr<Command> Construct(const protoMsg::CommandRequest& request) {
      static MockProcessClientCommand processClient(ConfSlave::Instance().GetConf());
      std::shared_ptr<MockUpgradeCommand> command(new MockUpgradeCommand(request,processClient));
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
   MockUpgradeCommand(const protoMsg::CommandRequest& request, ProcessClient& processClient) : UpgradeCommand(request,processClient) {}
   MockUpgradeCommand() = delete;
   MockUpgradeCommand(const MockUpgradeCommand& ) = delete;
};
