#include "CommandProcessorTests.h"
#include "Conf.h"
#include "MockConf.h"
#include "Headcrab.h"
#include "Crowbar.h"
#include "libconf/Conf.h"
#include "boost/lexical_cast.hpp"
#include "CommandReply.pb.h"
#include "CommandRequest.pb.h"


TEST_F(CommandProcessorTests, ConstructAndInitializeFail) {
   MockConf conf;
   conf.mCommandQueue = "invalid";
   CommandProcessor testProcessor(conf);
   EXPECT_FALSE(testProcessor.Initialize());
}
TEST_F(CommandProcessorTests, ConstructAndInitialize) {
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand()%1000 + 20000);
   CommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   raise(SIGTERM);
}
TEST_F(CommandProcessorTests, InvalidCommandSendReceive) {
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand()%1000 + 20000);
   CommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   Crowbar sender(conf.getCommandQueue());
   ASSERT_TRUE(sender.Wield());
   std::string requestMsg("ABC123");
   sender.Swing(requestMsg);
   std::string reply;
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   protoMsg::CommandReply replyMsg;
   replyMsg.ParseFromString(reply);
   EXPECT_FALSE(replyMsg.success());
   raise(SIGTERM);
}
TEST_F(CommandProcessorTests, CommandSendReceive) {
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand()%1000 + 20000);
   CommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   Crowbar sender(conf.getCommandQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::CommandRequest requestMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   sender.Swing(requestMsg.SerializeAsString());
   std::string reply;
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   protoMsg::CommandReply replyMsg;
   replyMsg.ParseFromString(reply);
   EXPECT_TRUE(replyMsg.success());
   raise(SIGTERM);
}
