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

TEST_F(ForkerPipeTest, CommandWithoutUUID) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   protoMsg::ForkerRequest requestProto;
   pid_t childPid(0);
   std::string commandId =clientPipe.CommandRequest(requestProto ,childPid);
   EXPECT_EQ(-1,childPid);
   EXPECT_TRUE(commandId.empty());
}
TEST_F(ForkerPipeTest, MakeDestoryUniqueFifos) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.abc"));
   EXPECT_FALSE(clientPipe.DestroyUniqueFifo("abc"));
   ASSERT_TRUE(clientPipe.MakeUniqueFifo("abc"));
   EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.abc"));
   EXPECT_FALSE(clientPipe.MakeUniqueFifo("abc"));
   EXPECT_TRUE(clientPipe.DestroyUniqueFifo("abc"));
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest.abc"));
}

TEST_F(ForkerPipeTest, GetTargetReceivePipe_GetTargetSendPipe) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   
   std::string serverSendPipe = serverPipe.GetTargetSendPipe();
   std::string clientSendPipe = clientPipe.GetTargetSendPipe();
   std::string serverReceiverPipe = serverPipe.GetTargetReceivePipe();
   std::string clientReceiverPipe = clientPipe.GetTargetReceivePipe();
   
   EXPECT_EQ(serverSendPipe,clientReceiverPipe);
   EXPECT_EQ(clientSendPipe,serverReceiverPipe);
   EXPECT_EQ("/tmp/ForkerPipeTest.serverToClient.fifo",serverSendPipe);
   EXPECT_EQ("/tmp/ForkerPipeTest.clientToServer.fifo",clientSendPipe);
}
TEST_F(ForkerPipeTest, GetUUID) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   
   std::string uuid = serverPipe.GetUUID();
   std::string client_uuid = clientPipe.GetUUID();
   EXPECT_NE(uuid,client_uuid); // verifies different seeds
   
   EXPECT_EQ(36,uuid.size());
   EXPECT_EQ(36,client_uuid.size());
}
TEST_F(ForkerPipeTest, ConstructUniquePipeName) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   
   std::string name = serverPipe.ConstructUniquePipeName("");
   EXPECT_EQ("/tmp/ForkerPipeTest",name);
   name = serverPipe.ConstructUniquePipeName("fubar");
   EXPECT_EQ("/tmp/ForkerPipeTest.fubar",name);
}
TEST_F(ForkerPipeTest, WaitForDataOnPipe) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   ASSERT_TRUE(clientPipe.MakeUniqueFifo("abc"));
   EXPECT_FALSE(serverPipe.WaitForDataOnPipe(0,1));
   std::thread clientThread([](MockForkerPipe& clientPipe) {    
      clientPipe.SendStringToUniquePipe("abc","def",30);
   }
   , std::ref(clientPipe));
   
   int pipe = serverPipe.ReadOpenUniqueFifo("abc");
   ASSERT_TRUE(serverPipe.WaitForDataOnPipe(pipe,10));
   std::string result = serverPipe.ReadFromReadyPipe(pipe);
   close(pipe);
   clientThread.join();
   EXPECT_EQ("def",result);
   EXPECT_TRUE(clientPipe.DestroyUniqueFifo("abc"));
}

TEST_F(ForkerPipeTest, ParseCommandProto) { 
   std::string command;
   std::vector<std::string> args;
   protoMsg::ForkerRequest requestProto;
   MockForkerPipe testPipe("ForkerPipeTest",false);
   
   EXPECT_FALSE(testPipe.ParseCommandProto(requestProto,command,args));
   requestProto.set_command("test");
   EXPECT_TRUE(testPipe.ParseCommandProto(requestProto,command,args));
   EXPECT_EQ("test",command);
   requestProto.add_args("arg1");
   requestProto.add_args("arg2");
   EXPECT_TRUE(testPipe.ParseCommandProto(requestProto,command,args));
   EXPECT_EQ("test",command);
   ASSERT_EQ(2,args.size());
   EXPECT_EQ("arg1",args[0]);
   EXPECT_EQ("arg2",args[1]);
   
}

TEST_F(ForkerPipeTest, CommandWithNoOutputExpected) {
   MockForkerPipe clientPipe("ForkerPipeTest");
   protoMsg::ForkerRequest requestProto;
   requestProto.set_expectreply(false);
   CommandState foo;
   foo.expectReply = false;
   foo.commandFinished = false;
   clientPipe.InsertDummyCommand("NoReply",foo);
   foo.expectReply = true;
   clientPipe.InsertDummyCommand("Reply",foo); 
   bool commandFinished(false);
   clientPipe.UpdateCommandResult("Reply", commandFinished);
   EXPECT_TRUE(commandFinished);
   clientPipe.UpdateCommandResult("NoReply", commandFinished);
   EXPECT_TRUE(commandFinished);
   
}

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
   
   int returnCode;
   bool gotReply = clientPipe.SendCommand("testCommand", args, result, id,returnCode,true);
   serverThread.join();
   EXPECT_FALSE(gotReply);
   
   std::thread clientThread([](MockForkerPipe& clientPipe, std::string& id, std::string& result, bool& gotReply) {
      int returnCode;
      while(!(gotReply=clientPipe.GetResultOfSentCommand(id,result,returnCode)));
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
   int returnCode;
   bool gotReply = clientPipe.SendCommand("testCommand", args, result, id,returnCode, true);
   while (!(gotReply=clientPipe.GetResultOfSentCommand(id,result,returnCode)));
   EXPECT_FALSE(id.empty());
   serverThread.join();
   EXPECT_EQ(requestProto.uuid(), id);
   EXPECT_EQ("success", result);
}
#endif