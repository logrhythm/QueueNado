#include "CommandFactoryTests.h"
#include "UpgradeCommand.h"

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
   delete upgradeCommand;
   factory.UnregisterCommand(protoMsg::CommandRequest_CommandType_UPGRADE);
   ASSERT_EQ(NULL,factory.GetCommand(request));
}