
#include "ConfProcessorTests.h"
#include "ConfNtp.h"
#include "ConfMaster.h"
#include "ConfSlave.h"
#include "MockConfMaster.h"
#include "MockConfSlave.h"
#include "MockConfNtpToReadFile.h"
#include "MockConfNtp.h"
#include "NtpMsg.pb.h"
#include <g2log.hpp>

using namespace std;
using namespace networkMonitor;

TEST_F(ConfProcessorTests, ConfNtp_Initialize) {
   ConfNtp conf;
   ASSERT_EQ("", conf.GetPath());
   ASSERT_EQ("/etc/ntp.conf", conf.GetNtpPath());
}

TEST_F(ConfProcessorTests, ConfNtp_InitializeWithProto) {
   protoMsg::Ntp msg;
   msg.set_master_server("127.0.0.1");
   msg.set_backup_server("");

   ::google::protobuf::Message* base = &msg;
   ConfNtp conf(*base);
   ASSERT_EQ(conf.GetMasterServer(), "127.0.0.1");
   std::string emtpy;
   ASSERT_EQ(conf.GetBackupServer(), emtpy);

   // Verify it works round-trip
   std::unique_ptr<protoMsg::Ntp> msg2(conf.getProtoMsg());
   ASSERT_EQ(msg2->master_server(), "127.0.0.1");
   ASSERT_TRUE(msg2->backup_server().empty());

}


TEST_F(ConfProcessorTests, DISABLED_REALREAD_ConfNtp_ReadFile) {
   MockConfNtpToReadFile conf;
   auto content = conf.GetFileContent();
   ASSERT_FALSE(content.empty());
}

namespace {
   static const std::string contentOK = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "server 10.128.64.251\n" \
        "server 10.128.64.252#\t\n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};
   
      static const std::string contentOKLongName = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "server 1234.567.8910.11.12.13.14.15.16.17.18.19.20 \n" \
        "server 0.centos.pool.ntp.whatever.org\n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};


   static const std::string contentWithNoBackup = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "server 10.128.64.251 \n" \
        "# server 10.128.64.252\n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};

   static const std::string contentCommentedOutServers = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "#server 10.128.64.251\n" \
        "# server 10.128.64.252\n" \
        "server\n" \
        "server \n" \
        "server # \n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};

   static const std::string contentWithNoServers = {"driftfile /var/lib/ntp/drift \n" \
        "restrict 127.0.0.1\n" \
        "restrict -6 ::1\n" \
        "server\n" \
        "server \n" \
        "server # \n" \
        "includefile /etc/ntp/crypto/pw\n" \
        "keys /etc/ntp/keys\n"};
}//anonymous

//only run these tests in debug because they require mocks.
#ifdef LR_DEBUG
TEST_F(ConfProcessorTests, ConfNtp_ReadContentsFromFile) {
   MockConfNtp conf(contentOK);
   ASSERT_EQ("10.128.64.251", conf.GetMasterServer());
   ASSERT_EQ("10.128.64.252", conf.GetBackupServer());
}

TEST_F(ConfProcessorTests, ConfNtp_ReadLongNamesFromFile) {
   MockConfNtp conf(contentOKLongName);
   ASSERT_EQ("1234.567.8910.11.12.13.14.15.16.17.18.19.20", conf.GetMasterServer());
   ASSERT_EQ("0.centos.pool.ntp.whatever.org", conf.GetBackupServer());
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

TEST_F(ConfProcessorTests, ConfNtp_WProtoBufUpdateTriggerWriteToFile) {
   MockConfNtp conf(contentWithNoServers);
   std::string master = {"10.128.64.251"};
   std::string backup = {"10.128.64.252"};

   ASSERT_EQ(conf.mContent.find(master), std::string::npos);
   ASSERT_EQ(conf.mContent.find(backup), std::string::npos);

   protoMsg::Ntp msg;
   msg.set_master_server(master);
   msg.set_backup_server(backup);
   conf.UpdateProtoMsg(msg);


   ASSERT_EQ(conf.GetMasterServer(), master);
   ASSERT_NE(conf.mContent.find(master), std::string::npos);
   ASSERT_EQ(conf.GetBackupServer(), backup);
   ASSERT_NE(conf.mContent.find(backup), std::string::npos);
}
#endif 

TEST_F(ConfProcessorTests, NtpMessagePassedBetweenMasterAndSlave) {
#if defined(LR_DEBUG)
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.Stop();
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   Conf conf(confThread.GetConf());
   MockConfSlave testSlave;
   testSlave.mBroadcastQueueName = conf.getBroadcastQueue();
   testSlave.Start();
   sleep(1);

   EXPECT_FALSE(testSlave.mNewNtpMsg);
   protoMsg::ConfType updateType;
   updateType.set_type(protoMsg::ConfType_Type_NTP);
   updateType.set_direction(protoMsg::ConfType_Direction_SENDING);
   protoMsg::Ntp confMsg;
   confMsg.set_master_server("10.128.64.251");
   std::vector<std::string> encodedMessage;

   encodedMessage.push_back(updateType.SerializeAsString());
   encodedMessage.push_back(confMsg.SerializeAsString());
   Crowbar confSender(conf.getConfChangeQueue());
   EXPECT_TRUE(confSender.Wield());
   EXPECT_TRUE(confSender.Flurry(encodedMessage));
   EXPECT_TRUE(confSender.BlockForKill(encodedMessage));
   EXPECT_EQ(2, encodedMessage.size());
   int sleepCount = 1;
   while (!testSlave.mNewNtpMsg && sleepCount <= 20) {
      sleep(1);
      sleepCount++;
   }
   EXPECT_TRUE(testSlave.mNewNtpMsg);
   testSlave.Stop();
   confThread.Stop();
#endif
}

TEST_F(ConfProcessorTests, TestReconcileNewNtpConf) {
#ifdef LR_DEBUG
   MockConfMaster master;
   ConfNtp conf;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_NTP);
   master.UpdateCachedMessage(conf);

   std::string serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   std::string message = master.SerializeCachedConfig(configTypeMessage);

   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf, message));
   master.Start();

   ConfNtp conf2(mWriteLocation);
   protoMsg::Ntp ntpMsg;
   EXPECT_TRUE(ntpMsg.ParseFromString(serializedConf));
   conf2.updateFields(ntpMsg);

   EXPECT_TRUE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   master.Stop();
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   master.Start();
   EXPECT_TRUE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   configTypeMessage.set_type(::protoMsg::ConfType_Type_RESTART);
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   configTypeMessage.set_type(::protoMsg::ConfType_Type_APP_VERSION);
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   message = "abc123";
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));

   master.Stop();
#endif
}



