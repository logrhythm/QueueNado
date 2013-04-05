#include "CommandFactoryTests.h"
#include "UpgradeCommand.h"
#include <g2loglevels.hpp>
#include "g2logworker.hpp"
#include "g2log.hpp"

TEST_F(CommandFactoryTests, ConstructNoRegsistered) {
   CommandFactory factory;
   protoMsg::CommandRequest request;
   request.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   ASSERT_EQ(NULL,factory.GetCommand(request));
}
TEST_F(CommandFactoryTests, ConstructRegsistered) {
   CommandFactory factory;
   protoMsg::CommandRequest request;
   request.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   
   ASSERT_EQ(NULL,factory.GetCommand(request));
   factory.RegisterCommand(protoMsg::CommandRequest_CommandType_UPGRADE, UpgradeCommand::Construct);
   Command* upgradeCommand = factory.GetCommand(request);
   ASSERT_TRUE(NULL!=upgradeCommand);
   LOG(INFO) << "Deleting upgrade command";
   delete upgradeCommand;
   LOG(INFO) << "Done";
   factory.UnregisterCommand(protoMsg::CommandRequest_CommandType_UPGRADE);
   ASSERT_EQ(NULL,factory.GetCommand(request));
}