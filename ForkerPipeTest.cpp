#include "ForkerPipeTest.h"
#include <memory>
#include <thread>
#include "FileIO.h"
#include "MockForkerMother.h"
#include "ClientPipe.h"
#include "ServerPipe.h"

TEST_F(ForkerPipeTest, Constructors) {
   {
      ServerPipe serverPipe("ForkerPipeTest");
      ClientPipe clientPipe("ForkerPipeTest");
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
   }
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
   {
      std::unique_ptr<ServerPipe> serverPipe(new ServerPipe("ForkerPipeTest"));
      std::unique_ptr<ClientPipe> clientPipe(new ClientPipe("ForkerPipeTest"));
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
      EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
   }
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo"));
}
#ifdef LR_DEBUG
TEST_F(ForkerPipeTest, GetResultOfSentCommand) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");
   protoMsg::ForkerRequest requestProto;
   std::vector<std::string> args;
   args.push_back("one");
   args.push_back("two");
   args.push_back("three");
   std::string result;
   ForkerPipe::CommandId id;
   requestProto.set_uuid("");
   std::thread serverThread([](MockServerPipe& serverPipe, protoMsg::ForkerRequest & requestProto) {
      if (serverPipe.GetCommand(requestProto, 60)) {
         protoMsg::ForkerReply replyProto;
         replyProto.set_success(true);
         replyProto.set_uuid(requestProto.uuid());
         replyProto.set_output("success");
      }

   }
   , std::ref(serverPipe), std::ref(requestProto));

   int returnCode;
   bool gotReply = clientPipe.SendCommand("testCommand", args, result, id, returnCode, true);
   serverThread.join();
   EXPECT_FALSE(gotReply);

   std::thread clientThread([](MockClientPipe& clientPipe, std::string& id, std::string& result, bool& gotReply) {
      int returnCode;
      while (!(gotReply = clientPipe.GetResultOfSentCommand(id, result, returnCode)));
      }

   , std::ref(clientPipe), std::ref(id), std::ref(result), std::ref(gotReply));

   protoMsg::ForkerReply replyProto;
   replyProto.set_success(true);
   replyProto.set_uuid(requestProto.uuid());
   replyProto.set_output("success");
   serverPipe.SendStringToUniquePipe(requestProto.uuid(), replyProto.SerializeAsString(), 1);
   clientThread.join();
   
   //bool finished;
   //serverPipe.UpdateCommandResult(requestProto.uuid(),finished);
   EXPECT_TRUE((gotReply));
   EXPECT_FALSE(id.empty());

   EXPECT_EQ(requestProto.uuid(), id);
   EXPECT_EQ("success", result);

   EXPECT_EQ(-1,unlink(serverPipe.ConstructUniquePipeName(id).c_str()));
}

TEST_F(ForkerPipeTest, GetCommandSendCommand) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");
   protoMsg::ForkerRequest requestProto;
   EXPECT_FALSE(serverPipe.GetCommand(requestProto, 1));

   std::vector<std::string> args;
   args.push_back("one");
   args.push_back("two");
   args.push_back("three");
   std::string result;
   ForkerPipe::CommandId id;
   requestProto.set_uuid("");
   std::thread serverThread([](MockServerPipe& serverPipe, protoMsg::ForkerRequest & requestProto) {
      if (serverPipe.GetCommand(requestProto, 60)) {
         protoMsg::ForkerReply replyProto;
         replyProto.set_success(true);
         replyProto.set_uuid(requestProto.uuid());
         replyProto.set_output("success");
         serverPipe.SendStringToUniquePipe(requestProto.uuid(), replyProto.SerializeAsString(), 1);
//         bool finished;
//         serverPipe.UpdateCommandResult(requestProto.uuid(),finished);
      }

   }
   , std::ref(serverPipe), std::ref(requestProto));
   int returnCode;
   bool gotReply = clientPipe.SendCommand("testCommand", args, result, id, returnCode, true);
   while (!(gotReply = clientPipe.GetResultOfSentCommand(id, result, returnCode)));
   EXPECT_FALSE(id.empty());
   serverThread.join();
   EXPECT_EQ(requestProto.uuid(), id);
   EXPECT_EQ("success", result);
   EXPECT_EQ(-1,unlink(serverPipe.ConstructUniquePipeName(id).c_str()));
}
TEST_F(ForkerPipeTest, CommandWithoutUUID) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");
   protoMsg::ForkerRequest requestProto;
   pid_t childPid(0);
   std::string commandId = serverPipe.CommandRequest(requestProto, childPid);
   EXPECT_EQ(-1, childPid);
   EXPECT_TRUE(commandId.empty());
}

TEST_F(ForkerPipeTest, MakeDestoryUniqueFifos) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.abc"));
   EXPECT_FALSE(clientPipe.DestroyUniqueFifo("abc"));
   ASSERT_TRUE(clientPipe.MakeUniqueFifo("abc"));
   EXPECT_TRUE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.abc"));
   EXPECT_FALSE(clientPipe.MakeUniqueFifo("abc"));
   EXPECT_TRUE(clientPipe.DestroyUniqueFifo("abc"));
   EXPECT_FALSE(FileIO::DoesFileExist("/tmp/ForkerPipeTest/ForkerPipeTest.abc"));
}

TEST_F(ForkerPipeTest, GetTargetReceivePipe_GetTargetSendPipe) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");

   std::string serverSendPipe = serverPipe.GetTargetSendPipe();
   std::string clientSendPipe = clientPipe.GetTargetSendPipe();
   std::string serverReceiverPipe = serverPipe.GetTargetReceivePipe();
   std::string clientReceiverPipe = clientPipe.GetTargetReceivePipe();

   EXPECT_EQ(serverSendPipe, clientReceiverPipe);
   EXPECT_EQ(clientSendPipe, serverReceiverPipe);
   EXPECT_EQ("/tmp/ForkerPipeTest/ForkerPipeTest.serverToClient.fifo", serverSendPipe);
   EXPECT_EQ("/tmp/ForkerPipeTest/ForkerPipeTest.clientToServer.fifo", clientSendPipe);
}

TEST_F(ForkerPipeTest, GetUUID) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");

   std::string uuid = serverPipe.GetUUID();
   std::string client_uuid = clientPipe.GetUUID();
   EXPECT_NE(uuid, client_uuid); // verifies different seeds

   EXPECT_EQ(36, uuid.size());
   EXPECT_EQ(36, client_uuid.size());
}

TEST_F(ForkerPipeTest, ConstructUniquePipeName) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");

   std::string name = serverPipe.ConstructUniquePipeName("");
   EXPECT_EQ("/tmp/ForkerPipeTest/ForkerPipeTest", name);
   name = serverPipe.ConstructUniquePipeName("fubar");
   EXPECT_EQ("/tmp/ForkerPipeTest/ForkerPipeTest.fubar", name);
}

TEST_F(ForkerPipeTest, WaitForDataOnPipe) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");
   ASSERT_TRUE(clientPipe.MakeUniqueFifo("abc"));
   EXPECT_FALSE(serverPipe.WaitForDataOnPipe(0, 1));
   std::thread clientThread([](MockServerPipe & serverPipe) {
      serverPipe.SendStringToUniquePipe("abc", "def", 30);
   }
   , std::ref(serverPipe));

   int pipe = clientPipe.ReadOpenUniqueFifo("abc");
   ASSERT_TRUE(clientPipe.WaitForDataOnPipe(pipe, 10));
   std::string result = clientPipe.ReadFromReadyPipe(pipe);
   close(pipe);
   clientThread.join();
   EXPECT_EQ("def", result);
   EXPECT_TRUE(clientPipe.DestroyUniqueFifo("abc"));
}

TEST_F(ForkerPipeTest, ParseCommandProto) {
   std::string command;
   std::vector<std::string> args;
   protoMsg::ForkerRequest requestProto;
   MockServerPipe testPipe("ForkerPipeTest");

   EXPECT_FALSE(testPipe.ParseCommandProto(requestProto, command, args));
   requestProto.set_command("test");
   EXPECT_TRUE(testPipe.ParseCommandProto(requestProto, command, args));
   EXPECT_EQ("test", command);
   requestProto.add_args("arg1");
   requestProto.add_args("arg2");
   EXPECT_TRUE(testPipe.ParseCommandProto(requestProto, command, args));
   EXPECT_EQ("test", command);
   ASSERT_EQ(2, args.size());
   EXPECT_EQ("arg1", args[0]);
   EXPECT_EQ("arg2", args[1]);

}



//
// Currently the timeout is 300 seconds, with retries every second
// This will push a lot of printouts to the system 
TEST_F(ForkerPipeTest, CommandWithNoOutputExpected_TakesTimeToFinish) {
   MockServerPipe serverPipe("ForkerPipeTest");
   MockClientPipe clientPipe("ForkerPipeTest");

   protoMsg::ForkerRequest requestProto;
   requestProto.set_expectreply(false);
   ForkerPipe::CommandState foo;
   foo.expectReply = false;
   foo.commandFinished = false;
   clientPipe.InsertDummyCommand("NoReply", foo);
   clientPipe.MakeUniqueFifo("NoReply");
   foo.expectReply = true;
   clientPipe.InsertDummyCommand("Reply", foo);
   clientPipe.MakeUniqueFifo("Reply");
   bool commandFinished(false);
   serverPipe.UpdateCommandResult("Reply", commandFinished);
   EXPECT_TRUE(commandFinished);
   serverPipe.UpdateCommandResult("NoReply", commandFinished);
   EXPECT_TRUE(commandFinished);
   std::string resultString;
   int returnCode;
   clientPipe.GetResultOfSentCommand("NoReply",resultString,returnCode);
   clientPipe.GetResultOfSentCommand("Reply",resultString,returnCode);
   EXPECT_FALSE(FileIO::DoesFileExist(clientPipe.ConstructUniquePipeName("Reply")));
   EXPECT_FALSE(FileIO::DoesFileExist(clientPipe.ConstructUniquePipeName("NoReply")));

}


#endif