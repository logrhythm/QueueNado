
#include "NtpConfigCommandTest.h"
#include "NtpConfigCommand.h"
#include "CommandReply.pb.h"
#include "ProcessManager.h"
#include "MockProcessManagerCommand.h"
#include "MockConf.h"
#include "NtpMsg.pb.h"
#include "MockNtpConfigCommand.h"
#include <sys/prctl.h>
#include <memory>


TEST_F(NtpConfigCommandTest, DoesItCompileAndLink) {
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   SUCCEED();
}

TEST_F(NtpConfigCommandTest, InValidCommand) {
   protoMsg::Ntp ntp;
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   auto reply = doIt.Execute(conf);
   ASSERT_FALSE(reply.success());
}

TEST_F(NtpConfigCommandTest, DisableNTP__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_active(false);
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());

   auto cmd = autoManagedManager->getRunCommand();
   auto cmdArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(cmd, std::string("service"));
   ASSERT_EQ(cmdArgs, std::string("ntpd stop"));

}

TEST_F(NtpConfigCommandTest, EnableNTPWithNoServer__ExpectingInvalidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_active(true);
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   auto reply = doIt.Execute(conf);
   ASSERT_FALSE(reply.success());
}

TEST_F(NtpConfigCommandTest, EnableNTPWithMasterServer__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_active(true);
   ntp.set_master_server("10.128.64.251");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());

   auto cmd = autoManagedManager->getRunCommand();
   auto cmdArgs = autoManagedManager->getRunArgs();
   ASSERT_EQ(cmd, std::string("service"));
   ASSERT_EQ(cmdArgs, std::string("ntpd restart"));

}

TEST_F(NtpConfigCommandTest, EnableNTPWithMasterAndServer__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_active(true);
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());
}

TEST_F(NtpConfigCommandTest, MultipleEnableCmds__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_active(true);
   ntp.set_master_server("10.128.64.251");
   ntp.set_backup_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());
}

TEST_F(NtpConfigCommandTest, MultipleDisableCmds__ExpectingValidCmd) {
   protoMsg::Ntp ntp;
   ntp.set_active(false);
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, autoManagedManager);
   for (int i = 0; i < 10; ++i) {
      auto reply = doIt.Execute(conf);
      ASSERT_TRUE(reply.success());
   }
} 

// Example run:
//sudo service ntpd stop; sudo service ntpd status; sudo ./test/CommandProcessorTests --gtest_filter=*Real*;sudo service ntpd statu
TEST_F(NtpConfigCommandTest, DISABLED_Real__Start__ExpectingFileChange) {
   protoMsg::Ntp ntp;
   ntp.set_active(true);
   ntp.set_master_server("10.128.64.252");
   cmd.set_stringargone(ntp.SerializeAsString());
   MockNtpConfigCommand doIt(cmd, new ProcessManager(conf)); //autoManagedManager); 
   auto reply = doIt.Execute(conf);
   ASSERT_TRUE(reply.success());
}

TEST_F(NtpConfigCommandTest, DISABLED_Execv__EnableNTP) {
   char* const realArgs[] = {"service", "ntpd restart", NULL};
   char *environ[] = {NULL};
   ASSERT_EQ(0, execve("/etc/init.d/ntpd", realArgs, environ));
}

TEST_F(NtpConfigCommandTest, DISABLED_Execv__DisableNTP) {
   char* const realArgs[] = {"service", "ntpd stop", NULL};
   char *environ[] = {NULL};
   ASSERT_EQ(0, execve("/etc/init.d/ntpd", realArgs, environ));
}

