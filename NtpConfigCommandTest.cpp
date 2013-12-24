
#include "NtpConfigCommandTest.h"
#include "NtpConfigCommand.h"
#include "CommandReply.pb.h"
#include "ProcessManager.h"
#include "MockProcessManagerCommand.h"
#include "MockCommandProcessor.h"
#include "MockConf.h"
#include "NtpMsg.pb.h"
#include "MockNtpConfigCommand.h"
#include "ConfNtp.h"
#include <sys/prctl.h>
#include <memory>
#include <boost/lexical_cast.hpp>


extern std::string gProgramName;
TEST_F(NtpConfigCommandTest, DoesItCompileAndLink) {
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   
   protoMsg::CommandRequest requestMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_NTP_CONFIG); 
   std::shared_ptr<Command> cmd = NtpConfigCommand::Construct(requestMsg,gProgramName);
   ASSERT_NE(cmd.get(), nullptr);
   SUCCEED();
}

TEST_F(NtpConfigCommandTest, Construct) {
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   SUCCEED();
}


TEST_F(NtpConfigCommandTest, InValidCommand) {
   protoMsg::Ntp ntp;
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   auto reply = doIt.Execute(conf);
   ASSERT_FALSE(reply.success());
}

TEST_F(NtpConfigCommandTest, DisableNTP__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_master_server(""); // only the master needs to be defined.. here with nothing
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());

   const auto& allCmds = autoManagedManager->getTotalRunCommands();   
   const auto& allArgs = autoManagedManager->getTotalRunArgs();
   ASSERT_EQ(allCmds.size(), allArgs.size());
   ASSERT_EQ(allCmds.size(), 0); // previously ntpd would stop for blank ntpd servers. That is no longer the case
      
   auto lastCmd = autoManagedManager->getRunCommand();
   auto lastArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(lastCmd, std::string(""));
   ASSERT_EQ(lastArgs, std::string(""));
   // Nothing to do since no servers were valid or active
}



TEST_F(NtpConfigCommandTest, EnableNTPWithNoServer__ExpectingInvalidCmd) {
   protoMsg::Ntp ntp;
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   auto reply = doIt.Execute(conf);
   ASSERT_FALSE(reply.success());
   
   const auto& allCmds = autoManagedManager->getTotalRunCommands();   
   const auto& allArgs = autoManagedManager->getTotalRunArgs();
   ASSERT_EQ(allCmds.size(), allArgs.size());
   ASSERT_EQ(allCmds.size(), 0);
   
   auto lastCmd = autoManagedManager->getRunCommand();
   auto lastArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(lastCmd, "");
   ASSERT_EQ(lastArgs, "");
}

TEST_F(NtpConfigCommandTest, EnableNTPWithMasterServer__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());

   const auto& allCmds = autoManagedManager->getTotalRunCommands();   
   const auto& allArgs = autoManagedManager->getTotalRunArgs();
   ASSERT_EQ(allCmds.size(), allArgs.size());
   ASSERT_EQ(allCmds.size(), 4);
   
   EXPECT_EQ(allCmds[0], "service");
   EXPECT_EQ(allArgs[0], "ntpd stop");

   EXPECT_EQ(allCmds[1], "ntpdate");
   EXPECT_EQ(allArgs[1], "-q 10.128.64.251");


   EXPECT_EQ(allCmds[2], "ntpdate");
   EXPECT_EQ(allArgs[2], "-s 10.128.64.251");


   EXPECT_EQ(allCmds[3], "service");
   EXPECT_EQ(allArgs[3], "ntpd start");
   
   
   auto cmd = autoManagedManager->getRunCommand();
   auto cmdArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(cmd, std::string("service"));
   ASSERT_EQ(cmdArgs, std::string("ntpd start"));
}

TEST_F(NtpConfigCommandTest, EnableNTPWithAliveMaster__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   doIt.oneServerAlive = {"10.128.64.251"};
 
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());

   const auto& allCmds = autoManagedManager->getTotalRunCommands();   
   const auto& allArgs = autoManagedManager->getTotalRunArgs();
   ASSERT_EQ(allCmds.size(), allArgs.size());
   ASSERT_EQ(allCmds.size(), 4); // no one extra 'ping' since master will succeed
   
   EXPECT_EQ(allCmds[0], "service");
   EXPECT_EQ(allArgs[0], "ntpd stop");

   EXPECT_EQ(allCmds[1], "ntpdate");
   EXPECT_EQ(allArgs[1], "-q 10.128.64.251"); 

   EXPECT_EQ(allCmds[2], "ntpdate");
   EXPECT_EQ(allArgs[2], "-s 10.128.64.251");

   EXPECT_EQ(allCmds[3], "service");
   EXPECT_EQ(allArgs[3], "ntpd start");
   
   auto cmd = autoManagedManager->getRunCommand();
   auto cmdArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(cmd, std::string("service"));
   ASSERT_EQ(cmdArgs, std::string("ntpd start"));
}


TEST_F(NtpConfigCommandTest, EnableNTPWithAliveBackupServer__ExpectingValidCmd) {
   #ifdef LR_DEBUG
    protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   doIt.oneServerAlive = {"10.128.64.252"};
 
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());

   const auto& allCmds = autoManagedManager->getTotalRunCommands();   
   const auto& allArgs = autoManagedManager->getTotalRunArgs();
   ASSERT_EQ(allCmds.size(), allArgs.size());
   ASSERT_EQ(allCmds.size(), 5); // one extra 'ping' since master will fail
   
   EXPECT_EQ(allCmds[0], "service");
   EXPECT_EQ(allArgs[0], "ntpd stop");

   EXPECT_EQ(allCmds[1], "ntpdate");
   EXPECT_EQ(allArgs[1], "-q 10.128.64.251"); // this failed

   EXPECT_EQ(allCmds[2], "ntpdate");
   EXPECT_EQ(allArgs[2], "-q 10.128.64.252");

   EXPECT_EQ(allCmds[3], "ntpdate");
   EXPECT_EQ(allArgs[3], "-s 10.128.64.252");


   EXPECT_EQ(allCmds[4], "service");
   EXPECT_EQ(allArgs[4], "ntpd start");
   
   auto cmd = autoManagedManager->getRunCommand();
   auto cmdArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(cmd, std::string("service"));
   ASSERT_EQ(cmdArgs, std::string("ntpd start"));
  #endif
}




TEST_F(NtpConfigCommandTest, EnableNTPWithAliveMasterAndAliveServer__ExpectingValidCmd) {
#ifdef LR_DEBUG   

   protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());
   
   const auto& allCmds = autoManagedManager->getTotalRunCommands();   
   const auto& allArgs = autoManagedManager->getTotalRunArgs();
   ASSERT_EQ(allCmds.size(), allArgs.size());
   ASSERT_EQ(allCmds.size(), 4);
   
   EXPECT_EQ(allCmds[0], "service");
   EXPECT_EQ(allArgs[0], "ntpd stop");

   EXPECT_EQ(allCmds[1], "ntpdate");
   EXPECT_EQ(allArgs[1], "-q 10.128.64.251");


   EXPECT_EQ(allCmds[2], "ntpdate");
   EXPECT_EQ(allArgs[2], "-s 10.128.64.251");


   EXPECT_EQ(allCmds[3], "service");
   EXPECT_EQ(allArgs[3], "ntpd start");
   
   auto lastCmd = autoManagedManager->getRunCommand();
   auto lastArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(lastCmd, std::string("service"));
   ASSERT_EQ(lastArgs, std::string("ntpd start"));
#endif
   
}

TEST_F(NtpConfigCommandTest, MultipleEnableCmds__ExpectingValidCmd) {
#ifdef LR_DEBUG

   protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());
#endif
}

TEST_F(NtpConfigCommandTest, MultipleDisableCmds__ExpectingValidCmd) {
#ifdef LR_DEBUG
   protoMsg::Ntp ntp;
   ntp.set_master_server("");
   ntp.set_backup_server("");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   for (int i = 0; i < 10; ++i) {
      auto reply = doIt.Execute(conf);
      ASSERT_TRUE(reply.success());
   }
#endif
} 

TEST_F(NtpConfigCommandTest, ThrowTestsIsServerAlive) {
#ifdef LR_DEBUG   
   protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   
   autoManagedManager->mSuccess = true;
   autoManagedManager->mReturnCode = 0;
   protoMsg::ProcessReply reply; 
   
   //Success IsServerAlive
   //    IsServerAlive should NEVER throw internally 
   EXPECT_NO_THROW(reply = doIt.IsServerAlive("10.128.64.251"));
   EXPECT_TRUE(reply.success() && (reply.returncode() ==0));
   EXPECT_NO_THROW(reply = doIt.IsServerAlive("10.128.64.252"));
   EXPECT_TRUE(reply.success() && (reply.returncode() ==0));
   
   // Failures
   autoManagedManager->mSuccess = false;
   autoManagedManager->mReturnCode = 0; // unlikely scenario. it would be the opposite
   EXPECT_NO_THROW(reply = doIt.IsServerAlive("10.128.64.251"));
   EXPECT_TRUE(!reply.success() && (reply.returncode() ==0));
   EXPECT_NO_THROW(doIt.IsServerAlive("10.128.64.252"));
   EXPECT_TRUE(!reply.success() || (reply.returncode() !=0));
   
   autoManagedManager->mSuccess = true;
   autoManagedManager->mReturnCode = 256; // likely scenario ntpdate -q fails with return "success"
   EXPECT_NO_THROW(reply = doIt.IsServerAlive("10.128.64.251"));
   EXPECT_TRUE(!reply.success() || (reply.returncode() !=0));
   EXPECT_NO_THROW(doIt.IsServerAlive("10.128.64.252"));
   EXPECT_TRUE(!reply.success() || (reply.returncode() !=0));
#endif
}

TEST_F(NtpConfigCommandTest, ThrowTestsForceTimeSync) {
 #ifdef LR_DEBUG  
   protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   
 
   protoMsg::ProcessReply reply; 
   
   // Sucess ForceTimeSync
   autoManagedManager->mSuccess = true;
   autoManagedManager->mReturnCode = 0;
   EXPECT_NO_THROW(reply = doIt.ForceTimeSync("10.128.64.251"));
   EXPECT_TRUE(reply.success() && (reply.returncode() ==0));
   
   // Failures
   autoManagedManager->mSuccess = false;
   autoManagedManager->mReturnCode = 0;
   EXPECT_NO_THROW(reply = doIt.ForceTimeSync("10.128.64.251"));
   EXPECT_TRUE(!reply.success() || (reply.returncode() !=0));

   // OR:  success but failed return code
   autoManagedManager->mSuccess = true;
   autoManagedManager->mReturnCode = 256;
   EXPECT_NO_THROW(reply = doIt.ForceTimeSync("10.128.64.251"));
   EXPECT_TRUE(!reply.success() || (reply.returncode() !=0));
#endif
}


TEST_F(NtpConfigCommandTest, ThrowTestsTriggerNtpdChange) {
#ifdef LR_DEBUG   
   protoMsg::Ntp ntp;
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager,gProgramName);
   
   //  for ntp a "failed" ntpdate -q  will still result in "success" but with nonzero return
   autoManagedManager->mSuccess = true;
   autoManagedManager->mReturnCode = 0;
   EXPECT_NO_THROW(doIt.TriggerNtpdChange());
   
   // Trigger throw from highLevel ntp change
   autoManagedManager->mSuccess = true;
   autoManagedManager->mReturnCode = 1; // same as failure
   EXPECT_ANY_THROW( doIt.TriggerNtpdChange());
   EXPECT_EQ(doIt.throwCounter, 0);

   // Trigger throw and count them   
   doIt.throwCounter = 0;
   doIt.willFakeThrow = true;
   EXPECT_NO_THROW( doIt.TriggerNtpdChange()); // throws are caught
   EXPECT_EQ(doIt.throwCounter, 2); // start + (delayed throw) ntpdate sync
#endif
}



//
// Do not execute this test or other "REAL" tests below it
// unless sure that you want to run a real ntpd change on your computer
TEST_F(NtpConfigCommandTest, DISABLED__REAL__NtpCommandSendReceive) {   
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);   
   MockCommandProcessor testProcessor(conf,gProgramName);
   EXPECT_TRUE(testProcessor.Initialize());
   
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_NTP_CONFIG, MockNtpConfigCommand::Construct);
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   Crowbar sender(conf.getCommandQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::CommandRequest requestMsg;
   requestMsg.set_type(protoMsg::CommandRequest_CommandType_NTP_CONFIG);   
   protoMsg::Ntp ntp;
   ntp.set_master_server("");
   requestMsg.set_stringargone(ntp.SerializeAsString());
   sender.Swing(requestMsg.SerializeAsString());
   std::string reply;
   sender.BlockForKill(reply);
   EXPECT_FALSE(reply.empty());
   protoMsg::CommandReply replyMsg;
   replyMsg.ParseFromString(reply);
   EXPECT_TRUE(replyMsg.success());
   raise(SIGTERM);
}


