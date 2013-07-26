
#include "NtpConfigCommandTest.h"
#include "NtpConfigCommand.h"
#include "CommandReply.pb.h"
#include "ProcessManager.h"
#include "MockProcessManagerCommand.h"
#include "MockConf.h"

#include "NtpMsg.pb.h"

#include <memory>
namespace {
struct MockNtpConfigCommand : public NtpConfigCommand {
  bool mSuccess;
  
  MockNtpConfigCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager) 
      : NtpConfigCommand(request, processManager), 
      mSuccess(true)  
  {   }
  
  ~MockNtpConfigCommand(){}
};
}



TEST_F(NtpConfigCommandTest, DoesItCompileAndLink) 
{
  MockNtpConfigCommand doIt(cmd, autoManagedManager);
  SUCCEED();
}

TEST_F(NtpConfigCommandTest, InValidCommand) 
{
  protoMsg::Ntp ntp;
  cmd.set_stringargone(ntp.SerializeAsString());
  MockNtpConfigCommand doIt(cmd, autoManagedManager);
  auto reply = doIt.Execute(conf);
  ASSERT_FALSE(reply.success());
}

TEST_F(NtpConfigCommandTest, NotActive__ValidCmd) 
{
  protoMsg::Ntp ntp;
  ntp.set_active(false);
  cmd.set_stringargone(ntp.SerializeAsString());
  MockNtpConfigCommand doIt(cmd, autoManagedManager);
  auto reply = doIt.Execute(conf);
  ASSERT_TRUE(reply.success());
}

TEST_F(NtpConfigCommandTest, ActiveWithNoServer__InValidCmd) 
{
  protoMsg::Ntp ntp;
  ntp.set_active(true);
  cmd.set_stringargone(ntp.SerializeAsString());
  MockNtpConfigCommand doIt(cmd, autoManagedManager);
  auto reply = doIt.Execute(conf);
  ASSERT_FALSE(reply.success());
}

TEST_F(NtpConfigCommandTest, ActiveWithMasterServer__ValidCmd) 
{
  protoMsg::Ntp ntp;
  ntp.set_active(true);
  ntp.set_master_server("10.128.64.251");
  cmd.set_stringargone(ntp.SerializeAsString());
  MockNtpConfigCommand doIt(cmd, autoManagedManager);
  auto reply = doIt.Execute(conf);
  ASSERT_TRUE(reply.success());
}

TEST_F(NtpConfigCommandTest, ActiveWithMasterAndServer__ValidCmd) 
{
  protoMsg::Ntp ntp;
  ntp.set_active(true);
  ntp.set_master_server("10.128.64.251");
  ntp.set_backup_server("10.128.64.252");
  cmd.set_stringargone(ntp.SerializeAsString());
  MockNtpConfigCommand doIt(cmd, autoManagedManager);
  auto reply = doIt.Execute(conf);
  ASSERT_TRUE(reply.success());
}