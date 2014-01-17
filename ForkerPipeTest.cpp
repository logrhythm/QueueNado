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


//LR_VIRTUAL bool GetStringFromPipeWithWait(std::string& resultString,const int waitInSeconds) {

//   LR_VIRTUAL bool GetStringFromUniquePipeWithWait(const CommandId& id,std::string& resultString,const int waitInSeconds){

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

//   LR_VIRTUAL int WriteOpenUniqueFifo(const CommandId& id) {
//   LR_VIRTUAL std::string GetTargetReceivePipe(){
//   LR_VIRTUAL std::string GetTargetSendPipe(){
//   LR_VIRTUAL std::string GetUUID(){
//   LR_VIRTUAL int RunExecVE(const char* command, char** arguments, char** environment){
//   LR_VIRTUAL bool IsChildLiving ( pid_t child ){

TEST_F(ForkerPipeTest, ConstructUniquePipeName) {
   MockForkerPipe serverPipe("ForkerPipeTest", false);
   MockForkerPipe clientPipe("ForkerPipeTest");
   
   std::string name = serverPipe.ConstructUniquePipeName("");
   EXPECT_EQ("/tmp/ForkerPipeTest",name);
   std::string name = serverPipe.ConstructUniquePipeName("fubar");
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