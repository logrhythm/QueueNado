#include "CommandFactoryTests.h"
#include "UpgradeCommand.h"
#include <g2loglevels.hpp>
#include "g2log.hpp"

TEST_F(CommandFactoryTests, ConstructNoRegsistered) {
   CommandFactory factory;
   protoMsg::CommandRequest request;
   request.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   std::shared_ptr<Command> notRegistered= factory.GetCommand(request);
   
   ASSERT_EQ(nullptr,notRegistered.get());
}
TEST_F(CommandFactoryTests, ConstructRegsistered) {
   CommandFactory factory;
   protoMsg::CommandRequest request;
   request.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   std::shared_ptr<Command> notRegistered= factory.GetCommand(request);
   ASSERT_EQ(nullptr,notRegistered.get());
   factory.RegisterCommand(protoMsg::CommandRequest_CommandType_UPGRADE, UpgradeCommand::Construct);
   std::shared_ptr<Command> upgradeCommand = factory.GetCommand(request);
   ASSERT_TRUE(nullptr!=upgradeCommand.get());
   LOG(INFO) << "Deleting upgrade command";
   LOG(INFO) << "Done";
   factory.UnregisterCommand(protoMsg::CommandRequest_CommandType_UPGRADE);
   notRegistered= factory.GetCommand(request);
   
   ASSERT_EQ(nullptr,notRegistered.get());
}