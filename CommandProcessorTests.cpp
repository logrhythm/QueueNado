#include "CommandProcessorTests.h"
#include "Conf.h"
#include "UpgradeCommandTest.h"
#include "NetworkConfigCommandTest.h"
#include "MockConf.h"
#include "Headcrab.h"
#include "Crowbar.h"
#include "libconf/Conf.h"
#include "boost/lexical_cast.hpp"
#include "CommandReply.pb.h"
#include "MockCommandProcessor.h"
#include "MockUpgradeCommand.h"
#include "CommandRequest.pb.h"
#include "MockProcessManagerCommand.h"
#include "CommandFailedException.h"
#include "QosmosDpiTest.h"
#include "RebootCommand.h"
#include "RebootCommandTest.h"
#include <g2loglevels.hpp>
#include "g2logworker.hpp"
#include "g2log.hpp"
#include "RestartSyslogCommandTest.h"
#include "NetInterfaceMsg.pb.h"

TEST_F(CommandProcessorTests, ConstructAndInitializeFail) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mCommandQueue = "invalid";
   CommandProcessor testProcessor(conf);
   EXPECT_FALSE(testProcessor.Initialize());
#endif
}

TEST_F(CommandProcessorTests, ConstructAndInitialize) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   CommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   raise(SIGTERM);
#endif
}

TEST_F(CommandProcessorTests, InvalidCommandSendReceive) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
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
#endif
}

TEST_F(CommandProcessorTests, CommandSendReceive) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockCommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   testProcessor.ChangeRegistration(protoMsg::CommandRequest_CommandType_UPGRADE, MockUpgradeCommand::Construct);
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
#endif
}

//Upgrade commands

TEST_F(CommandProcessorTests, UpgradeCommandInit) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
}


TEST_F(CommandProcessorTests, UpgradeCommandExecSuccess) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = upg.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
}

TEST_F(CommandProcessorTests, DynamicUpgradeCommandExecSuccess) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest* upg = new UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = upg->Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }
   delete upg;
   ASSERT_FALSE(exception);
}


TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCodeCreatePassPhrase) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.CreatePassPhraseFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessCreatePassPhrase) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.CreatePassPhraseFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCodeDecryptFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.DecryptFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessDecryptFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.DecryptFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCodeRenameDecryptedFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.RenameDecryptedFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessRenameDecryptedFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.RenameDecryptedFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCodeUntarFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.UntarFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessUntarFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.UntarFile();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnRunUpgradeScript) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.RunUpgradeScript();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessRunUpgradeScript) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.RunUpgradeScript();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailReturnCleanUploadDir) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.CleanUploadDir();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, UpgradeCommandFailSuccessCleanUploadDir) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      upg.CleanUploadDir();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}


TEST_F(CommandProcessorTests, UpgradeCommandFailInitProcessManager) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->setInit(false);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_UPGRADE);
   cmd.set_stringargone("filename");
   UpgradeCommandTest upg = UpgradeCommandTest(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply  reply = upg.Execute(conf);
      ASSERT_FALSE(reply.success());     
      
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_FALSE(exception);
}

//REBOOT COMMANDS

TEST_F(CommandProcessorTests, RebootCommandExecSuccess) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RebootCommandTest reboot = RebootCommandTest(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = reboot.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
}

TEST_F(CommandProcessorTests, RebootCommandFailReturnDoTheUpgrade) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RebootCommandTest reboot = RebootCommandTest(cmd, processManager);
   bool exception = false;
   try {
      reboot.DoTheReboot();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, RebootCommandFailSuccessDoTheUpgrade) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RebootCommandTest reboot = RebootCommandTest(cmd, processManager);
   bool exception = false;
   try {
      reboot.DoTheReboot();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

// Syslog Restart Commands

TEST_F(CommandProcessorTests, RestartSyslogCommandExecSuccess) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot = RestartSyslogCommandTest(cmd, processManager);
   bool exception = false;
   try {
      protoMsg::CommandReply reply = reboot.Execute(conf);
      LOG(DEBUG) << "Success: " << reply.success() << " result: " << reply.result();
      ASSERT_TRUE(reply.success());
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
}

TEST_F(CommandProcessorTests, RestartSyslogCommandTestFailRestart) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot = RestartSyslogCommandTest(cmd, processManager);
   bool exception = false;
   try {
      reboot.Restart();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

TEST_F(CommandProcessorTests, RestartSyslogCommandTestFailSuccessRestart) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_REBOOT);
   RestartSyslogCommandTest reboot = RestartSyslogCommandTest(cmd, processManager);
   bool exception = false;
   try {
      reboot.Restart();
   } catch(CommandFailedException e) {
      exception = true;
   }
   ASSERT_TRUE(exception);
}

// Network Config commands

TEST_F(CommandProcessorTests, NetworkConfigCommandInit) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
}

TEST_F(CommandProcessorTests, NetworkConfigCommandExecSuccess) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::DHCP);
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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

TEST_F(CommandProcessorTests, DynamicNetworkConfigCommandExecSuccess) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
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
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   cmd.set_stringargone("BadInterfaceMsg");
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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

TEST_F(CommandProcessorTests, NetworkConfigCommandFailInitProcessManager) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->setInit(false);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::DHCP);
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(0);
   processManager->SetResult("Success!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_ipaddress("192.168.1.1");
   interfaceConfig.set_netmask("255.255.255.0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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

TEST_F(CommandProcessorTests, NetworkConfigFailIfcfgInterfaceAllowedEth2) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("eth2");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("em3");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
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
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.BackupIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/cat", processManager->getRunCommand());
   ASSERT_EQ( "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\" > "
              "\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\"",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessBackupIfcfgFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.BackupIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/cat", processManager->getRunCommand());
   ASSERT_EQ( "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\" > "
              "\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\"",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeRestoreIfcfgFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.RestoreIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/cat", processManager->getRunCommand());
   ASSERT_EQ( "\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\" > "
              "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessRestoreIfcfgFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.RestoreIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/cat", processManager->getRunCommand());
   ASSERT_EQ( "\"/etc/sysconfig/network-scripts/bkup-ifcfg-NoIface\" > "
              "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeResetIfcfgFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.ResetIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/usr/bin/perl", processManager->getRunCommand());
   ASSERT_EQ( "-ni -e 'print unless /BOOTPROTO|IPADDR|NETMASK|GATEWAY|NETWORK|"
              "NM_CONTROLLED|ONBOOT|DNS1|DNS2|PEERDNS|DOMAIN|BOARDCAST/i' "
              "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessResetIfcfgFile) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("NoIface");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.ResetIfcfgFile();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/usr/bin/perl", processManager->getRunCommand());
   ASSERT_EQ( "-ni -e 'print unless /BOOTPROTO|IPADDR|NETMASK|GATEWAY|NETWORK|"
              "NM_CONTROLLED|ONBOOT|DNS1|DNS2|PEERDNS|DOMAIN|BOARDCAST/i' "
              "\"/etc/sysconfig/network-scripts/ifcfg-NoIface\"",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddBootProto) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::DHCP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddBootProto("dhcp");
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"BOOTPROTO=dhcp\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddBootProto) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddBootProto("none");
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"BOOTPROTO=none\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailAddIpAddrNotDefined) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddIpAddr();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "", processManager->getRunCommand());
   ASSERT_EQ( "", processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddIpAddr) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_ipaddress("192.168.1.1");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddIpAddr();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"IPADDR=192.168.1.1\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddIpAddr) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_ipaddress("192.168.1.1");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddIpAddr();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"IPADDR=192.168.1.1\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandAddNetmaskNotDefined) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddNetmask();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "", processManager->getRunCommand());
   ASSERT_EQ( "", processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddNetmask) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_netmask("255.255.255.0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddNetmask();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"NETMASK=255.255.255.0\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddNetmask) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_netmask("255.255.255.0");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddNetmask();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"NETMASK=255.255.255.0\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailAddGatewayNotDefined) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddGateway();
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
   ASSERT_EQ( "", processManager->getRunCommand());
   ASSERT_EQ( "", processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddGateway) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_gateway("192.168.1.100");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddGateway();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"GATEWAY=192.168.1.100\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddGateway) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_gateway("192.168.1.100");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddGateway();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"GATEWAY=192.168.1.100\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDnsServers) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDnsServers();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DNS1=192.168.1.10\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDnsServers) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDnsServers();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DNS1=192.168.1.10\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDns1) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDns1();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DNS1=192.168.1.10\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDns1) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDns1();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DNS1=192.168.1.10\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDns2) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDns2();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DNS2=192.168.1.11\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDns2) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDns2();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DNS2=192.168.1.11\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddDomain) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_searchdomains("schq.secious.com");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDomain();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DOMAIN=schq.secious.com\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddDomain) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   interfaceConfig.set_searchdomains("schq.secious.com");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddDomain();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"DOMAIN=schq.secious.com\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreReturnCodeInterfaceDown) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceDown();
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifdown", processManager->getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreSuccessInterfaceDown) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceDown();
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifdown", processManager->getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreReturnCodeInterfaceUp) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceUp();
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifup", processManager->getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandIgnoreSuccessInterfaceUp) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.InterfaceUp();
   } catch (...) {
      exception = true;
   }
   ASSERT_FALSE(exception);
   ASSERT_EQ("/sbin/ifup", processManager->getRunCommand());
   ASSERT_EQ("ethx boot --force", processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddOnBoot) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddOnBoot();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"ONBOOT=yes\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddOnBoot) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddOnBoot();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"ONBOOT=yes\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddNmControlled) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddNmControlled();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"NM_CONTROLLED=no\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddNmControlled) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddNmControlled();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"NM_CONTROLLED=no\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailReturnCodeAddPeerDns) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(true);
   processManager->SetReturnCode(1);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   // No DNS Servers or Search Domains set, which causes PEERDNS=no on output
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddPeerDns();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"PEERDNS=no\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

TEST_F(CommandProcessorTests, NetworkConfigCommandFailSuccessAddPeerDns) {
   const MockConf conf;
   MockProcessManagerCommand* processManager = new MockProcessManagerCommand(conf);
   processManager->SetSuccess(false);
   processManager->SetReturnCode(0);
   processManager->SetResult("Failed!");
   protoMsg::CommandRequest cmd;
   cmd.set_type(protoMsg::CommandRequest_CommandType_NETWORK_CONFIG);
   protoMsg::NetInterface interfaceConfig;
   interfaceConfig.set_method(protoMsg::STATICIP);
   interfaceConfig.set_interface("ethx");
   // Set a DNS Servers, which causes PEERDNS=yes on output
   interfaceConfig.set_dnsservers("192.168.1.10,192.168.1.11");
   cmd.set_stringargone(interfaceConfig.SerializeAsString());
   NetworkConfigCommandTest ncct = NetworkConfigCommandTest(cmd, processManager);
   bool exception = false;
   try {
      ncct.AddPeerDns();
   } catch (...) {
      exception = true;
   }
   ASSERT_TRUE(exception);
   ASSERT_EQ( "/bin/echo", processManager->getRunCommand());
   ASSERT_EQ( "\"PEERDNS=yes\" >> /etc/sysconfig/network-scripts/ifcfg-ethx",
              processManager->getRunArgs());
}

