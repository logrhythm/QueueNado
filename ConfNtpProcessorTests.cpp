
#include "ConfProcessorTests.h"
#include "ConfNtp.h"
#include "NtpMsg.pb.h"
#include <g2log.hpp>

TEST_F(ConfProcessorTests, ConfNtp_Initialize) {
  ConfNtp conf;
  ASSERT_EQ("conf/nm.yaml.Interface", conf.GetPath());
  ASSERT_EQ("/etc/ntp.conf", conf.GetNtpPath());
}

TEST_F(ConfProcessorTests, ConfNtp_InitializeWithProto) {
  protoMsg::Ntp msg;
  msg.set_active(true);
  msg.set_master_server("127.0.0.1");
  
  ::google::protobuf::Message* base = &msg;
  ConfNtp conf(*base);
  ASSERT_TRUE(conf.GetEnabled());
  ASSERT_EQ(conf.GetMasterServer(), "127.0.0.1");
  ASSERT_TRUE(conf.GetBackupServer().empty());
  
  // Verify it works round-trip
  std::unique_ptr<protoMsg::Ntp> msg2(conf.getProtoMsg());
  ASSERT_TRUE(msg2->active());
  ASSERT_EQ(msg2->master_server(), "127.0.0.1");
  ASSERT_TRUE(msg2->backup_server().empty());
  
}


TEST_F(ConfProcessorTests, ConfNtp) {
  ConfNtp conf;
  ASSERT_EQ("conf/nm.yaml.Interface", conf.GetPath());
  ASSERT_EQ("/etc/ntp.conf", conf.GetNtpPath());
}




namespace {

  struct MockConfNtpToReadFile : public ConfNtp {

    MockConfNtpToReadFile() : ConfNtp() {
    }

    std::string GetFileContent() override {
      return ConfNtp::GetFileContent();
    }
  };
}

TEST_F(ConfProcessorTests, ConfNtp_ReadFile) {
  MockConfNtpToReadFile conf;
  auto content = conf.GetFileContent();
  ASSERT_FALSE(content.empty());
}

namespace {

  struct MockConfNtp : public ConfNtp {
    std::string mContent;

    explicit MockConfNtp(const std::string content) : ConfNtp(), mContent(content) {
      ReadNtpConfFromFile();
    }
  protected:
    std::string GetFileContent() override {
      LOG(INFO) << "content is: " << mContent;
      return mContent;
    }
    
    void WriteServersToFile(const std::string& content) override {
      mContent = content;
    }
    
  };

  const std::string contentOK = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "server 10.128.64.251\n" \
        "server 10.128.64.252\n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};


  const std::string contentWithNoBackup = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "server 10.128.64.251\n" \
        "# server 10.128.64.252\n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};
  
      const std::string contentCommentedOutServers = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "#server 10.128.64.251\n" \
        "# server 10.128.64.252\n" \
        "server\n" \
        "server \n" \
        "server # \n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};
      
    const std::string contentWithNoServers = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "server\n" \
        "server \n" \
        "server # \n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};
}//anonymous

TEST_F(ConfProcessorTests, ConfNtp_ReadContentsFromFile) {
  MockConfNtp conf(contentOK);
  ASSERT_EQ("10.128.64.251", conf.GetMasterServer());
  ASSERT_EQ("10.128.64.252", conf.GetBackupServer());
}

TEST_F(ConfProcessorTests, ConfNtp_ReadContentsFromFileWithCommentedOutBackup) {
  MockConfNtp conf(contentWithNoBackup);
  ASSERT_EQ("10.128.64.251", conf.GetMasterServer());
  ASSERT_EQ("", conf.GetBackupServer()); 
}

TEST_F(ConfProcessorTests, ConfNtp_ReadContentsFromFileWithNoServers) {
  MockConfNtp conf(contentCommentedOutServers);
  std::string empty;
  ASSERT_EQ(empty, conf.GetMasterServer());
  ASSERT_EQ(empty, conf.GetBackupServer()); 
}

TEST_F(ConfProcessorTests, ConfNtp_WriteToFile) {  
  MockConfNtp conf(contentWithNoServers);
  std::string master = {"10.128.64.251"};
  std::string backup = {"10.128.64.252"};
 
  ASSERT_EQ(conf.mContent.find(master), std::string::npos);
  conf.UpdateMasterServer(master);
  ASSERT_EQ(conf.GetMasterServer(), master);
  ASSERT_NE(conf.mContent.find(master), std::string::npos);
  
  ASSERT_NE(conf.GetBackupServer(), backup);
  ASSERT_EQ(conf.mContent.find(backup), std::string::npos);
  conf.UpdateBackupServer(backup);
  ASSERT_EQ(conf.GetBackupServer(), backup);  
  ASSERT_NE(conf.mContent.find(backup), std::string::npos);
}


TEST_F(ConfProcessorTests, ConfNtp_ProtoBufUpdateTriggerWriteToFile) {  
  MockConfNtp conf(contentWithNoServers);
  std::string master = {"10.128.64.251"};
  std::string backup = {"10.128.64.252"};
 
  ASSERT_EQ(conf.mContent.find(master), std::string::npos);
  ASSERT_EQ(conf.mContent.find(backup), std::string::npos);
  
  protoMsg::Ntp msg;
  msg.set_active(true);
  msg.set_master_server(master);
  msg.set_backup_server(backup);
  conf.UpdateProtoMsg(msg);
 
  
  ASSERT_EQ(conf.GetMasterServer(), master);
  ASSERT_NE(conf.mContent.find(master), std::string::npos);
  ASSERT_EQ(conf.GetBackupServer(), backup);  
  ASSERT_NE(conf.mContent.find(backup), std::string::npos);
}
