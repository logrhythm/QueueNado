#include "ForkerPipeTest.h"
#include <memory>
#include <thread>
#include "FileIO.h"

TEST_F(ForkerPipeTest, Constructors) {
   {
      ForkerPipe serverPipe("ForkerPipeTest", false);
      ForkerPipe clientPipe("ForkerPipeTest");
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
   }
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
   {
      std::unique_ptr<ForkerPipe> serverPipe(new ForkerPipe("ForkerPipeTest", false));
      std::unique_ptr<ForkerPipe> clientPipe(new ForkerPipe("ForkerPipeTest"));
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
   }
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.serverToClient.fifo"));
}
#ifdef LR_DEBUG
TEST_F(ForkerPipeTest, GetResultOfSentCommand) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   protoMsg::ForkerRequest requestProto;
   std::vector<std::string> args;
   args.push_back("one");
   args.push_back("two");
   args.push_back("three");
   std::string result;
   ForkerPipe::CommandId id;
   requestProto.set_uuid("");
   std::thread serverThread([](MockForkerPipe& serverPipe, protoMsg::ForkerRequest & requestProto) {
      if (serverPipe.GetCommand(requestProto, 60)) {
         protoMsg::ForkerReply replyProto;
         replyProto.set_success(true);
         replyProto.set_uuid(requestProto.uuid());
         replyProto.set_output("success");
      }

   }
   , std::ref(serverPipe), std::ref(requestProto));
   
   bool gotReply = clientPipe.SendCommand("testCommand", args, result, id);
   serverThread.join();
   EXPECT_FALSE(gotReply);
   
   std::thread clientThread([](MockForkerPipe& clientPipe, std::string& id, std::string& result, bool& gotReply) {
      while(!(gotReply=clientPipe.GetResultOfSentCommand(id,result)));
   }
   , std::ref(clientPipe), std::ref(id), std::ref(result), std::ref(gotReply));
   
   protoMsg::ForkerReply replyProto;
   replyProto.set_success(true);
   replyProto.set_uuid(requestProto.uuid());
   replyProto.set_output("success");
   serverPipe.SendStringToUniquePipe(requestProto.uuid(),replyProto.SerializeAsString(), 1);
   clientThread.join();
   EXPECT_TRUE((gotReply));
   EXPECT_FALSE(id.empty());
   
   EXPECT_EQ(requestProto.uuid(), id);
   EXPECT_EQ("success", result);
}
TEST_F(ForkerPipeTest, GetCommandSendCommand) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   protoMsg::ForkerRequest requestProto;
   EXPECT_FALSE(serverPipe.GetCommand(requestProto, 1));

   std::vector<std::string> args;
   args.push_back("one");
   args.push_back("two");
   args.push_back("three");
   std::string result;
   ForkerPipe::CommandId id;
   requestProto.set_uuid("");
   std::thread serverThread([](MockForkerPipe& serverPipe, protoMsg::ForkerRequest & requestProto) {
      if (serverPipe.GetCommand(requestProto, 60)) {
         protoMsg::ForkerReply replyProto;
         replyProto.set_success(true);
         replyProto.set_uuid(requestProto.uuid());
         replyProto.set_output("success");
         serverPipe.SendStringToUniquePipe(requestProto.uuid(),replyProto.SerializeAsString(), 1);
      }

   }
   , std::ref(serverPipe), std::ref(requestProto));
   
   bool gotReply = clientPipe.SendCommand("testCommand", args, result, id);
   while (!(gotReply=clientPipe.GetResultOfSentCommand(id,result)));
   EXPECT_FALSE(id.empty());
   serverThread.join();
   EXPECT_EQ(requestProto.uuid(), id);
   EXPECT_EQ("success", result);
}
#endif