#include "CommandProcessorTests.h"
#include "ConfigRequestCommand.h"
#include "Conf.h"
#include "UpgradeCommandTest.h"
#include "NetworkConfigCommandTest.h"
#include "MockConf.h"
#include "Headcrab.h"
#include "Crowbar.h"
#include "libconf/Conf.h"
#include "boost/lexical_cast.hpp"
#include "CommandReply.pb.h"
#include "MockUpgradeCommand.h"
#include "CommandRequest.pb.h"
#include "MockProcessClientCommand.h"
#include "CommandFailedException.h"
#include "QosmosDpiTest.h"
#include "RebootCommand.h"
#include "NtpConfigCommand.h"
#include "RebootCommandTest.h"
#include "MockShutdownCommand.h"
#include <g2loglevels.hpp>
#include "g2log.hpp"
#include "RestartSyslogCommandTest.h"
#include "NetInterfaceMsg.pb.h"
#include "ShutdownMsg.pb.h"
#include "MockTestCommand.h"
#include "FileIO.h"


bool CommandProcessorTests::mDeathReceived = false;
std::string CommandProcessorTests::mDeathMessage = {};

#ifdef LR_DEBUG
//
//  The Mock Shutdown sets a flag if the DoTheShutdown gets triggered. 
//  As long as the Mock is not tampered with and the ChangeRegistration below is in order
//  this test will NOT shutdown your PC, only fake it. 
//
//  If you tamper with the details mentioned, all bets are OFF!
//

TEST_F(CommandProcessorTests, PseudoShutdown) {
#ifdef LR_DEBUG

   MockCommandProcessor* testProcessor;
   {
      MockConf conf;
      conf.mCommandQueue = "tcp://127.0.0.1:";
      conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
      MockCommandProcessor testProcessor{conf};
      EXPECT_TRUE(testProcessor.Initialize());
      LOG(INFO) << "Executing Real command with real Processor but with Mocked Shutdown function";
      // NEVER CHANGE the LINE below. If it is set to true your PC will shut down
      MockShutdownCommand::callRealShutdownCommand = false;
      MockShutdownCommand::wasShutdownCalled = false;

      testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_SHUTDOWN, MockShutdownCommand::FatalAndDangerousConstruct);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      Crowbar sender(conf.GetCommandQueue());
      ASSERT_TRUE(sender.Wield());
      protoMsg::CommandRequest requestMsg;
      requestMsg.set_type(protoMsg::CommandRequest_CommandType_SHUTDOWN);
      protoMsg::ShutdownMsg shutdown;
      shutdown.set_now(true);
      requestMsg.set_stringargone(shutdown.SerializeAsString());

      sender.Swing(requestMsg.SerializeAsString());
      std::string reply;
      sender.BlockForKill(reply);
      EXPECT_FALSE(reply.empty());
      protoMsg::CommandReply replyMsg;
      replyMsg.ParseFromString(reply);
      EXPECT_TRUE(replyMsg.success());
      EXPECT_TRUE(MockShutdownCommand::wasShutdownCalled);
   }


#endif
}


// Starts an Async command, Gets its running status, kills the command
// Tries to get the running status which will fail since the command is removed
TEST_F(CommandProcessorTests, StartAQuickAsyncCommandAndGetStatusDontGetStatus) {

   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_TEST, MockTestCommandRunsForever::Construct);

   Crowbar sender(conf.GetCommandQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::CommandRequest requestMsg;
   unsigned int count(0);
   std::string reply;

   protoMsg::CommandReply replyMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_TEST);
   requestMsg.set_async(true);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());

   replyMsg.ParseFromString(reply);
   EXPECT_TRUE(replyMsg.success());
   // Expecing UUID
   const size_t sizeOfUUID = 36;
   const std::string uuid = replyMsg.result();
   EXPECT_NE(uuid, "Command running") << " : " << uuid;
   EXPECT_EQ(uuid.size(), sizeOfUUID) << uuid;
   
   // Next request for STATUS should receive "Command running"
   requestMsg.set_type(::protoMsg::CommandRequest_CommandType_COMMAND_STATUS);
   requestMsg.set_async(false);
   requestMsg.set_stringargone(uuid);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   protoMsg::CommandReply intermediateReply;
   intermediateReply.ParseFromString(reply);
   EXPECT_FALSE(intermediateReply.success());
   EXPECT_EQ(intermediateReply.result(), "Command running") << " : " << intermediateReply.result();

   
   // Trigger a "KillCommandsThatWillNeverFinish" before retrieving the command
   testProcessor.SetTimeout(0);
   WaitForKillCommandsThatWillNeverFinish(testProcessor);

   requestMsg.set_type(::protoMsg::CommandRequest_CommandType_COMMAND_STATUS);
   requestMsg.set_async(false);
   requestMsg.set_stringargone(uuid);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   protoMsg::CommandReply realReply;
   realReply.ParseFromString(reply);
   EXPECT_FALSE(realReply.success());
   EXPECT_EQ(realReply.result(), "Command Not Found") << " : " << realReply.result();
}

TEST_F(CommandProcessorTests, StartAQuickAsyncCommandAndGetStatusForcedKill) {


   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_TEST, MockTestCommandRunsForever::Construct);

   Crowbar sender(conf.GetCommandQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::CommandRequest requestMsg;
   unsigned int count(0);
   std::string reply;
   protoMsg::CommandReply realReply;
   protoMsg::CommandReply replyMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_TEST);
   requestMsg.set_async(true);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());

   replyMsg.ParseFromString(reply);
   EXPECT_TRUE(replyMsg.success());
   count = 0;

   requestMsg.set_type(::protoMsg::CommandRequest_CommandType_COMMAND_STATUS);
   requestMsg.set_async(false);
   requestMsg.set_stringargone(replyMsg.result());
   do {
      requestMsg.set_stringargone(replyMsg.result());
      sender.Swing(requestMsg.SerializeAsString());
      std::string reply;
      sender.BlockForKill(reply);
      EXPECT_FALSE(reply.empty());
      realReply.ParseFromString(reply);
      if (realReply.has_completed() && realReply.completed()) {
         break;
      } else {
         EXPECT_TRUE(realReply.result() == "Command running");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   } while (!zctx_interrupted && count++ < 100);
      
   // Trigger a "KillCommandsThatWillNeverFinish" before retrieving the command
   testProcessor.SetTimeout(1);
   WaitForKillCommandsThatWillNeverFinish(testProcessor);
   
   std::this_thread::sleep_for(std::chrono::milliseconds(2001));
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_FALSE(realReply.success());
   EXPECT_TRUE(realReply.result() == "Command Not Found");

}

TEST_F(CommandProcessorTests, ExecuteForkTests) {
   protoMsg::CommandRequest requestMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_TEST);
   requestMsg.set_async(true);
   std::shared_ptr<Command> holdMe(MockTestCommand::Construct(requestMsg));
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   std::shared_ptr < std::atomic<bool> > threadRefSet = std::make_shared < std::atomic<bool> >(false);

   unsigned int count(0);
   std::weak_ptr<Command> weakCommand(holdMe);
   pthread_t threadID; 
   Command::ExecuteFork(holdMe, conf, threadID, threadRefSet);

   while (!*threadRefSet && !zctx_interrupted && count++ < 1000) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
   ASSERT_TRUE(1000 > count); // the thread ownership shouldn't take longer than a second
   EXPECT_TRUE(holdMe.use_count() > 1); // the thread has at least incremented the count by one (possibly 2)
   count = 0;
   while (!std::dynamic_pointer_cast<MockTestCommand>(holdMe)->Finished() && !zctx_interrupted && count++ < 1000) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
   ASSERT_TRUE(1000 > count); // the completion shouldn't take longer than a second
   EXPECT_TRUE(std::dynamic_pointer_cast<MockTestCommand>(holdMe)->GetResult().success());
   EXPECT_TRUE(std::dynamic_pointer_cast<MockTestCommand>(holdMe)->GetResult().result() == "TestCommand");
   ASSERT_TRUE(std::dynamic_pointer_cast<MockTestCommand>(holdMe)->GetResult().has_completed());
   EXPECT_TRUE(std::dynamic_pointer_cast<MockTestCommand>(holdMe)->GetResult().completed());
   EXPECT_TRUE(holdMe.use_count() == 2); // the reference count stays +1 till GetStatus succeeds
   protoMsg::CommandReply reply = Command::GetStatus(weakCommand);
   EXPECT_TRUE(reply.success() == std::dynamic_pointer_cast<MockTestCommand>(holdMe)->GetResult().success());
   EXPECT_TRUE(reply.result() == std::dynamic_pointer_cast<MockTestCommand>(holdMe)->GetResult().result());
   EXPECT_TRUE(reply.completed() == std::dynamic_pointer_cast<MockTestCommand>(holdMe)->GetResult().completed());
   EXPECT_TRUE(holdMe.use_count() == 1); // the thread has dropped all but one reference
}

TEST_F(CommandProcessorTests, GetStatusTests) {
   protoMsg::CommandRequest requestMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_TEST);
   requestMsg.set_async(true);
   std::shared_ptr<Command> holdMe(MockTestCommand::Construct(requestMsg));
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);

   std::weak_ptr<Command> weakCommand(holdMe);

   protoMsg::CommandReply reply = Command::GetStatus(weakCommand);
   EXPECT_FALSE(reply.success());
   EXPECT_FALSE(reply.has_completed());

   holdMe.reset();

   reply = Command::GetStatus(weakCommand);
   EXPECT_TRUE(reply.success());
   EXPECT_TRUE(reply.has_completed());
   EXPECT_TRUE(reply.completed());
   EXPECT_EQ("Result Already Sent", reply.result());
}

TEST_F(CommandProcessorTests, StartAQuickAsyncCommandAndGetStatusAlwaysFails) {

   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_TEST, MockTestCommandAlwaysFails::Construct);

   Crowbar sender(conf.GetCommandQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::CommandRequest requestMsg;
   unsigned int count(0);
   std::string reply;
   protoMsg::CommandReply realReply;
   protoMsg::CommandReply replyMsg;
   for (int i = 0; i < 100; i++) {
      requestMsg.set_type(protoMsg::CommandRequest_CommandType_TEST);
      requestMsg.set_async(true);
      sender.Swing(requestMsg.SerializeAsString());
      sender.BlockForKill(reply);
      EXPECT_FALSE(reply.empty());

      replyMsg.ParseFromString(reply);
      EXPECT_TRUE(replyMsg.success());
      count = 0;

      requestMsg.set_type(::protoMsg::CommandRequest_CommandType_COMMAND_STATUS);
      requestMsg.set_async(false);
      requestMsg.set_stringargone(replyMsg.result());
      do {

         requestMsg.set_stringargone(replyMsg.result());
         sender.Swing(requestMsg.SerializeAsString());
         std::string reply;
         sender.BlockForKill(reply);
         EXPECT_FALSE(reply.empty());
         realReply.ParseFromString(reply);
         if (realReply.has_completed() && realReply.completed()) {
            break;
         } else {
            EXPECT_EQ(realReply.result(), "Command running");
         }
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      } while (!zctx_interrupted && count++ < 100);
      EXPECT_EQ(realReply.result(), "TestCommandFails");
      EXPECT_FALSE(realReply.success());
   }

 
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_TRUE(realReply.success());
   EXPECT_EQ(realReply.result(), "Result Already Sent");

   WaitForKillCommandsThatWillNeverFinish(testProcessor);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_FALSE(realReply.success());
   EXPECT_EQ(realReply.result(), "Command Not Found");

}

TEST_F(CommandProcessorTests, StartAQuickAsyncCommandAndGetStatusExitApp) {


   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_TEST, MockTestCommandRunsForever::Construct);

   Crowbar sender(conf.GetCommandQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::CommandRequest requestMsg;
   unsigned int count(0);
   std::string reply;
   protoMsg::CommandReply realReply;
   protoMsg::CommandReply replyMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_TEST);
   requestMsg.set_async(true);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());

   replyMsg.ParseFromString(reply);
   EXPECT_TRUE(replyMsg.success());
   raise(SIGTERM);
   std::this_thread::sleep_for(std::chrono::milliseconds(1001));
}

TEST_F(CommandProcessorTests, StartAQuickAsyncCommandAndGetStatus) {

   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_TEST, MockTestCommand::Construct);

   Crowbar sender(conf.GetCommandQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::CommandRequest requestMsg;
   unsigned int count(0);
   std::string reply;
   protoMsg::CommandReply realReply;
   protoMsg::CommandReply replyMsg;
   for (int i = 0; i < 100; i++) {
      requestMsg.set_type(protoMsg::CommandRequest_CommandType_TEST);
      requestMsg.set_async(true);
      sender.Swing(requestMsg.SerializeAsString());
      sender.BlockForKill(reply);
      EXPECT_FALSE(reply.empty());

      replyMsg.ParseFromString(reply);
      EXPECT_TRUE(replyMsg.success());
      count = 0;

      requestMsg.set_type(::protoMsg::CommandRequest_CommandType_COMMAND_STATUS);
      requestMsg.set_async(false);
      requestMsg.set_stringargone(replyMsg.result());
      do {

         requestMsg.set_stringargone(replyMsg.result());
         sender.Swing(requestMsg.SerializeAsString());
         std::string reply;
         sender.BlockForKill(reply);
         EXPECT_FALSE(reply.empty());
         realReply.ParseFromString(reply);
         if (realReply.has_completed() && realReply.completed()) {
            break;
         } else {
            EXPECT_EQ(realReply.result(), "Command running");
         }
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      } while (!zctx_interrupted && count++ < 100);
      EXPECT_EQ(realReply.result(), "TestCommand");
      EXPECT_TRUE(realReply.success());
   }

   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_TRUE(realReply.success());
   auto result = realReply.result();
   EXPECT_TRUE(result == "Result Already Sent"); 
   
   WaitForKillCommandsThatWillNeverFinish(testProcessor);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_FALSE(realReply.success());
   EXPECT_EQ(realReply.result(), "Command Not Found");

}

TEST_F(CommandProcessorTests, CommandStatusFailureTests) {

   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   Crowbar sender(conf.GetCommandQueue());
   ASSERT_TRUE(sender.Wield());
   std::string reply;
   protoMsg::CommandReply realReply;
   protoMsg::CommandRequest requestMsg;

   requestMsg.set_type(::protoMsg::CommandRequest_CommandType_COMMAND_STATUS);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_FALSE(realReply.success());
   EXPECT_TRUE("Invalid Status Request, No ID" == realReply.result());
   requestMsg.set_stringargone("abc123");
   requestMsg.set_async(true);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_FALSE(realReply.success());
   EXPECT_TRUE("Invalid Status Request, Cannot Process Asynchronously" == realReply.result());
   requestMsg.set_async(false);
   sender.Swing(requestMsg.SerializeAsString());
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   realReply.ParseFromString(reply);
   EXPECT_FALSE(realReply.success());
   EXPECT_TRUE("Command Not Found" == realReply.result());

}
#endif

TEST_F(CommandProcessorTests, ConstructAndInitialize) {
#ifdef LR_DEBUG

   CommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());

#endif
}

TEST_F(CommandProcessorTests, ConstructAndInitializeCheckRegistrations) {
#ifdef LR_DEBUG

   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   EXPECT_EQ(UpgradeCommand::Construct, testProcessor.CheckRegistration(protoMsg::CommandRequest_CommandType_UPGRADE));
   EXPECT_EQ(RestartSyslogCommand::Construct, testProcessor.CheckRegistration(protoMsg::CommandRequest_CommandType_SYSLOG_RESTART));
   EXPECT_EQ(RebootCommand::Construct, testProcessor.CheckRegistration(protoMsg::CommandRequest_CommandType_REBOOT));
   EXPECT_EQ(NetworkConfigCommand::Construct, testProcessor.CheckRegistration(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG));
   EXPECT_EQ(NtpConfigCommand::Construct, testProcessor.CheckRegistration(protoMsg::CommandRequest_CommandType_NTP_CONFIG));
   EXPECT_EQ(ShutdownCommand::Construct, testProcessor.CheckRegistration(protoMsg::CommandRequest_CommandType_SHUTDOWN));
   EXPECT_EQ(ConfigRequestCommand::Construct, testProcessor.CheckRegistration(protoMsg::CommandRequest_CommandType_CONFIG_REQUEST));
#endif
}

TEST_F(CommandProcessorTests, InvalidCommandSendReceive) {
#ifdef LR_DEBUG

   CommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   Crowbar sender(conf.GetCommandQueue());
   ASSERT_TRUE(sender.Wield());
   std::string requestMsg("ABC123");
   sender.Swing(requestMsg);
   std::string reply;
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   protoMsg::CommandReply replyMsg;
   replyMsg.ParseFromString(reply);
   EXPECT_FALSE(replyMsg.success());

#endif
}

TEST_F(CommandProcessorTests, CommandSendReceive) {
#ifdef LR_DEBUG

   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_UPGRADE, MockUpgradeCommand::Construct);
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   Crowbar sender(conf.GetCommandQueue());
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

#endif
}

//Upgrade commands

TEST_F(CommandProcessorTests, UpgradeCommandInit) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandExecSuccess) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   bool ignoreAnyFileCreateExceptions = true;
   UpgradeCommandTest upg(cmd, processManager, ignoreAnyFileCreateExceptions);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = upg.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success()) << reply.success() << " result: " << reply.result();
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
#endif
}

TEST_F(CommandProcessorTests, DynamicUpgradeCommandExecSuccess) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   bool noThrowForFileWriteError = true;
   UpgradeCommandTest* upg = new UpgradeCommandTest(cmd, processManager, noThrowForFileWriteError);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = upg->Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success()) << reply.success() << " result: " << reply.result();
   } catch (...) {
      exception = true;
   }
   delete upg;
   ASSERT_FALSE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailCreatePassPhrase) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   upg.SetBadPassPhraseFilePath();
   bool exception = false;
   try {
      upg.CreatePassPhraseFile();
   } catch (CommandFailedException& e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif 
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCodeDecryptFile) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.DecryptFile();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessDecryptFile) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.DecryptFile();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCodeRenameDecryptedFile) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.RenameDecryptedFile();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessRenameDecryptedFile) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.RenameDecryptedFile();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCodeUntarFile) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.UntarFile();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessUntarFile) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.UntarFile();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnRunUpgradeScript) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.RunUpgradeScript();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessRunUpgradeScript) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.RunUpgradeScript();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCleanUploadDir) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.CleanUploadDir();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessCleanUploadDir) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg(cmd, processManager);
   bool exception = false;
   try {
      upg.CleanUploadDir();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}



//REBOOT COMMANDS

TEST_F(CommandProcessorTests, RebootCommandExecSuccess) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RebootCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = reboot.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
      EXPECT_TRUE(processManager.mRunCommand == "/sbin/init");
      EXPECT_TRUE(processManager.mRunArgs == " 6");
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
#endif
}

TEST_F(CommandProcessorTests, ShutdownCommandExecSuccess) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_SHUTDOWN);
   MockShutdownCommand shutdown(cmd, processManager);
   MockShutdownCommand::callRealShutdownCommand = true;
   bool exception = false;
   try {
      protoMsg::CommandReply reply = shutdown.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
      EXPECT_EQ(processManager.mRunCommand, "/sbin/init");
      EXPECT_EQ(processManager.mRunArgs, " 0");
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
#endif
}

TEST_F(CommandProcessorTests, RebootCommandFailReturnDoTheUpgrade) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RebootCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      reboot.DoTheReboot();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, RebootCommandFailSuccessDoTheUpgrade) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RebootCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      reboot.DoTheReboot();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

// Syslog Restart Commands

TEST_F(CommandProcessorTests, RestartSyslogCommandExecSuccess) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mSyslogConfName = "/tmp/test.nm.rsyslog.conf"; 
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = reboot.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      EXPECT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }
   unlink(conf.mSyslogConfName.c_str());
   ASSERT_FALSE(exception);
   

#endif
}

TEST_F(CommandProcessorTests, RestartSyslogCommandExecSuccess_UDP) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mSyslogConfName = "/ThisFileDoesNotExisttmp/test.nm.rsyslog.conf"; // dummy file. won't be created
   conf.mSyslogProtocol = false; // udp
   conf.mSyslogAgentIp = "123.123.123";
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      reboot.UpdateSyslog(conf);
   } catch (CommandFailedException& e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, RestartSyslogCommandExecSuccess_TCP) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mSyslogConfName = "/ThisFileDoesNotExisttmp/test.nm.rsyslog.conf"; // dummy file. won't be created
   conf.mSyslogProtocol = true; // "TCP";
   conf.mSyslogAgentIp = "123.123.123";
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      reboot.UpdateSyslog(conf);
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
#endif
}

TEST_F(CommandProcessorTests, RestartSyslogCommandTestFailedUpdate) {
#ifdef LR_DEBUG
   MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      reboot.UpdateSyslog(conf);
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);

#endif
}

TEST_F(CommandProcessorTests, RestartSyslogCommandTestFailRestart) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      reboot.Restart();
   } catch (CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);

#endif
}

TEST_F(CommandProcessorTests, RestartSyslogCommandTestFailSuccessRestart) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot(cmd, processManager);
   bool exception = false;
   try {
      reboot.Restart();
   } catch (CommandFailedException& e) {
      exception = true;
   }
   ASSERT_TRUE(exception);

#endif
}

// Network Config commands

TEST_F(CommandProcessorTests, NetworkConfigCommandInit) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   NetworkConfigCommandTest ncct(cmd, processManager);

#endif 
}

TEST_F(CommandProcessorTests, NetworkConfigCommandExecSuccess) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::DHCP);
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = ncct.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);

#endif
}

TEST_F(CommandProcessorTests, DynamicNetworkConfigCommandExecSuccess) {
#ifdef LR_DEBUG
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::DHCP);
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest* ncct = new NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = ncct->Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }
   delete ncct;
   ASSERT_FALSE(exception);


}

TEST_F(CommandProcessorTests, NetworkConfigCommandBadInterfaceMsg) {

   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   cmd.set_stringargone("BadInterfaceMsg");
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = ncct.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_FALSE(reply.success());
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);


}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailInterfaceMethodNotSet) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = ncct.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_FALSE(reply.success());
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandSetStaticIpSuccess) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_ipaddress("192.168.1.1");
   interfaceConfig.set_netmask("255.255.255.0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = ncct.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }

   ASSERT_FALSE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigFailIfcfgInterfaceAllowedEth1) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("eth1");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.setStatFail();
   bool exception = false;
   try {
      ncct.IfcfgInterfaceAllowed();
   } catch (...) {
      exception = true;
   }

   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigFailIfcfgInterfaceAllowedEm2) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("em2");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.setStatFail();
   bool exception = false;
   try {
      ncct.IfcfgInterfaceAllowed();
   } catch (...) {
      exception = true;
   }

   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigFailIfcfgInterfaceAllowedEth2) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("eth2");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.setStatFail();
   bool exception = false;
   try {
      ncct.IfcfgInterfaceAllowed();
   } catch (...) {
      exception = true;
   }

   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigFailIfcfgInterfaceAllowedEm3) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("em3");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.setStatFail();
   bool exception = false;
   try {
      ncct.IfcfgInterfaceAllowed();
   } catch (...) {
      exception = true;
   }

   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigFailIfcfgFileExists) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.setStatFail();
   bool exception = false;
   try {
      ncct.IfcfgFileExists();
   } catch (...) {
      exception = true;
   }

   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeBackupIfcfgFile) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.BackupIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ("/bin/sh", processManager.getRunCommand());

   ASSERT_EQ("/bin/cat \"/etc/sysconfig/network-scripts/ifcfg-NoIface\" > "
           "\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\"",
           FileIO::ReadAsciiFileContent(processManager.getRunArgs()).result);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessBackupIfcfgFile) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.BackupIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ("/bin/sh", processManager.getRunCommand());
   ASSERT_EQ("/bin/cat \"/etc/sysconfig/network-scripts/ifcfg-NoIface\" > "
           "\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\"",
           FileIO::ReadAsciiFileContent(processManager.getRunArgs()).result);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeRestoreIfcfgFile) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.RestoreIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ("/bin/cat", processManager.getRunCommand());
   ASSERT_EQ("\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\" > "
           "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
           processManager.getRunArgs());

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessRestoreIfcfgFile) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.RestoreIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ("/bin/cat", processManager.getRunCommand());
   ASSERT_EQ("\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\" > "
           "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
           processManager.getRunArgs());

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeResetIfcfgFile) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.ResetIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ("/usr/bin/perl", processManager.getRunCommand());
   ASSERT_EQ("-ni -e 'print unless /BOOTPROTO|IPADDR|NETMASK|GATEWAY|NETWORK|"
           "NM_CONTROLLED|ONBOOT|DNS1|DNS2|PEERDNS|DOMAIN|BOARDCAST/i' "
           "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
           processManager.getRunArgs());

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessResetIfcfgFile) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.ResetIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ("/usr/bin/perl", processManager.getRunCommand());
   ASSERT_EQ("-ni -e 'print unless /BOOTPROTO|IPADDR|NETMASK|GATEWAY|NETWORK|"
           "NM_CONTROLLED|ONBOOT|DNS1|DNS2|PEERDNS|DOMAIN|BOARDCAST/i' "
           "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
           processManager.getRunArgs());

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailAddBootProto) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::DHCP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddBootProto("dhcp");
   } catch (CommandFailedException& e) {
      exception = true;
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("BOOTPROTO=dhcp"));
   }
   ASSERT_TRUE(exception);


}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailAddIpAddrNotDefined) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddIpAddr();
   } catch (CommandFailedException& e) {
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailAddIpAddrEmptyString) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_ipaddress("");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddIpAddr();
   } catch (CommandFailedException& e) {
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddIpAddr) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_ipaddress("192.168.1.1");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddIpAddr();
   } catch (CommandFailedException& e) {
      exception = true;

      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("IPADDR=192.168.1.1")) << whatItWas;
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()))<< whatItWas;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddIpAddr) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_ipaddress("192.168.1.1");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddIpAddr();
   } catch (CommandFailedException& e) {
      exception = true;
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("IPADDR=192.168.1.1"))<< whatItWas;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandAddNetmaskNotDefined) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddNetmask();
   } catch (CommandFailedException& e) {

      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandAddNetmaskEmptyString) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_netmask("");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddNetmask();
   } catch (CommandFailedException& e) {

      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddNetmask) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_netmask("255.255.255.0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddNetmask();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("NETMASK=255.255.255.0"))<< whatItWas;
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()))<< whatItWas;
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddNetmask) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_netmask("255.255.255.0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddNetmask();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("NETMASK=255.255.255.0"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailAddGatewayNotDefined) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddGateway();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      exception = true;
   }
   ASSERT_FALSE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddGateway) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_gateway("192.168.1.100");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddGateway();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("GATEWAY=192.168.1.100"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));

      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddGateway) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_gateway("192.168.1.100");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddGateway();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("GATEWAY=192.168.1.100"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandAddGatewayEmptyString) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_gateway("");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddGateway();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   ASSERT_FALSE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDnsServers) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDnsServers();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("DNS1=192.168.1.10"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDnsServers) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDnsServers();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("DNS1=192.168.1.10"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandAddDnsServersEmptyString) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDnsServers();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   ASSERT_FALSE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDns1) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDns1();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("DNS1=192.168.1.10"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDns1) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDns1();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("DNS1=192.168.1.10"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDns2) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDns2();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("DNS2=192.168.1.11"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDns2) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDns2();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("DNS2=192.168.1.11"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDomain) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_searchdomains("schq.secious.com");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDomain();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("DOMAIN=schq.secious.com"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDomain) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_searchdomains("schq.secious.com");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddDomain();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("DOMAIN=schq.secious.com"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandAddDomainEmptyString) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_searchdomains("");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDomain();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   ASSERT_FALSE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreReturnCodeInterfaceDown) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceDown();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifdown", processManager.getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager.getRunArgs());

}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreSuccessInterfaceDown) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceDown();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   EXPECT_EQ(processManager.mCountNumberOfRuns, 3); // 3x ifup
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifdown", processManager.getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager.getRunArgs());

}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreReturnCodeInterfaceUp) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceUp();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   EXPECT_EQ(processManager.mCountNumberOfRuns, 3); // 3x  ifup
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifup", processManager.getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager.getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreSuccessInterfaceUp) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceUp();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   EXPECT_EQ(processManager.mCountNumberOfRuns, 3); // 3x ifup
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifup", processManager.getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager.getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandStaticNoExtraRetriesOnSuccessfulInterfaceUp) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("eth0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceUp();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   EXPECT_EQ(processManager.mCountNumberOfRuns, 1); // 1 ifup
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifup", processManager.getRunCommand());
   ASSERT_EQ("eth0 boot --force", processManager.getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandDhcpNoExtraRetriesOnSuccessfulInterfaceUp) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(0);
   processManager.SetResult("");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::DHCP);
   interfaceConfig.set_interface("eth0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceUp();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      exception = true;
   }
   EXPECT_EQ(processManager.mCountNumberOfRuns, 1); // 1 ifup
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifup", processManager.getRunCommand());
   ASSERT_EQ("eth0 boot --force", processManager.getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddOnBoot) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddOnBoot();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("ONBOOT=yes"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);


}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddOnBoot) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddOnBoot();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());
      EXPECT_NE(std::string::npos, whatItWas.find("ONBOOT=yes"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddNmControlled) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddNmControlled();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("NM_CONTROLLED=no"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddNmControlled) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddNmControlled();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("NM_CONTROLLED=no"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddPeerDns) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   // No DNS Servers or Search Domains set, which causes PEERDNS=no on output
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddPeerDns();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());


      EXPECT_NE(std::string::npos, whatItWas.find("PEERDNS=no"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandDnsServerEmptyStringSearchDomainNo) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("");
   // DNS Server is empty string and no Search Domains set, which causes PEERDNS=no on output
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddPeerDns();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("PEERDNS=no"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandDnsServerNoSearchDomainEmptyString) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_searchdomains("");
   // No DNS Server and Search Domains is empty string, which causes PEERDNS=no on output
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddPeerDns();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("PEERDNS=no"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandDnsServerSearchDomainEmptyStrings) {
   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(true);
   processManager.SetReturnCode(1);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("");
   interfaceConfig.set_searchdomains("");
   // DNS Server and Search Domains are both empty strings, which causes PEERDNS=no on output
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddPeerDns();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("PEERDNS=no"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddPeerDns) {

   const MockConf conf;
   MockProcessClientCommand processManager{conf};
   processManager.SetSuccess(false);
   processManager.SetReturnCode(0);
   processManager.SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   // Set a DNS Servers, which causes PEERDNS=yes on output
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   interfaceConfig.set_searchdomains("");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct(cmd, processManager);
   ncct.ManglemIfcfgFile();
   bool exception = false;
   try {
      ncct.AddPeerDns();
   } catch (CommandFailedException& e) {
      std::string whatItWas(e.what());

      EXPECT_NE(std::string::npos, whatItWas.find("PEERDNS=yes"));
      EXPECT_NE(std::string::npos, whatItWas.find(ncct.GetIfcfgFile()));
      exception = true;
   }
   ASSERT_TRUE(exception);

#endif
}

// THIS DEATH TEST SHOULD BE LAST of all the tests in CommandProcessorTests since it will effectively CHANGE 
// How G2log will deal with CHECK and LOG(FATAL)
TEST_F(CommandProcessorTests, DEATH_TEST__ConstructAndInitializeFail) {
#ifdef LR_DEBUG

   g2::internal::changeFatalInitHandlerForUnitTesting(CommandProcessorTests::DeathReceiver);
   EXPECT_FALSE(CommandProcessorTests::mDeathReceived);
   EXPECT_TRUE(CommandProcessorTests::mDeathMessage.find("Cannot start command reader listener queue") == std::string::npos);
   { // explicit scope for thread cleanup
      MockConf conf;
      conf.mCommandQueue = "invalid";
      CommandProcessor testProcessor(conf);
      ASSERT_NO_THROW(testProcessor.Initialize()) << "Test assert should be caught by the Unit test Fatal handler";
      std::this_thread::sleep_for(std::chrono::seconds(2));
   } // scope exit will make sure that the thread was executed and cleaned up


   EXPECT_TRUE(CommandProcessorTests::mDeathReceived);
   EXPECT_TRUE(CommandProcessorTests::mDeathMessage.find("Cannot start command reader listener queue") != std::string::npos);
#endif
}
