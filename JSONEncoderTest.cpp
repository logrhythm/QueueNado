#include <stdlib.h>
#include "JSONEncoder.h"
#include "JSONEncoderTest.h"
#include "g2logworker.hpp"
#include "g2log.hpp"
#include "DpiMsgLR.h"

TEST_F(JSONEncoderTest, Construct) {
   networkMonitor::DpiMsgLR testMsg;

   JSONEncoder encoder(&testMsg);
   JSONEncoder* pEncoder = new JSONEncoder(&testMsg);
   delete pEncoder;
}

TEST_F(JSONEncoderTest, EncodeAnEmptyMessage) {
   networkMonitor::DpiMsgLR testMsg;

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.Encode();

   EXPECT_EQ("{\"uuid\": \"\"}", encodedMessage);
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAString) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.Encode();

   EXPECT_EQ("{\"uuid\": \"ABC123\"}", encodedMessage);
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_destport(123);
   std::string encodedMessage = encoder.Encode();

   EXPECT_EQ("{\"destPort\": 123, \"uuid\": \"ABC123\"}", encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithIpSrcDst) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_ipsrc(0x11223344);
   testMsg.set_ipdst(0x55667788);
   std::string encodedMessage = encoder.Encode();

   EXPECT_EQ("{\"uuid\": \"ABC123\", \"ipDst\": \"136.119.102.85\", \"ipSrc\": \"68.51.34.17\"}", encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt64) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_deltatime(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_EQ("{\"deltaTime\": 12345678900, \"uuid\": \"ABC123\"}", encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithATimeT) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_starttime(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_EQ("{\"uuid\": \"ABC123\", \"startTime\": \"2361/03/22 19:15:00\"}", encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithEthSrcDst) {
      networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_ethsrc(12345678900L);
   testMsg.set_ethdst(12345678901L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_EQ("{\"uuid\": \"ABC123\", \"ethDst\": \"35:1c:dc:df:02:00\", \"ethSrc\": \"34:1c:dc:df:02:00\"}", encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithRepeatedStringField) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   testMsg.add_accept_encodingq_proto_http("1");
   testMsg.add_accept_encodingq_proto_http("2");
   testMsg.add_accept_encodingq_proto_http("3");

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_EQ("{\"uuid\": \"ABC123\", \"Engine_acceptEncoding\": [\"1\", \"2\", \"3\"]}", encodedMessage);
}

TEST_F(JSONEncoderTest, EncodeAMessageWithRenamer) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   testMsg.add_uri_fullq_proto_http("1");
   testMsg.add_application_endq_proto_base("test");
   testMsg.set_application_id_endq_proto_base(1234);
   testMsg.set_sessionlenserver(567);
   testMsg.set_deltasessionlenserver(567);
   testMsg.set_sessionlenclient(899);
   testMsg.set_deltasessionlenclient(899);
   testMsg.set_packetcount(88);
   testMsg.set_deltapackets(88);
   testMsg.add_loginq_proto_aim("aLogin");
   testMsg.add_domainq_proto_smb("aDomain12345");
   testMsg.add_uriq_proto_http("not/this/one");
   testMsg.add_serverq_proto_http("thisname12345");
   testMsg.add_referer_serverq_proto_http("notThisOne");
   testMsg.add_methodq_proto_ftp("RUN");
   testMsg.add_methodq_proto_ftp("COMMAND");
   testMsg.add_methodq_proto_ftp("LONGLONGLONGLONG");
   testMsg.add_senderq_proto_smtp("test1_123456");
   testMsg.add_receiverq_proto_smtp("test2_123");
   testMsg.add_subjectq_proto_smtp("test3_12345");
   testMsg.add_versionq_proto_http("4.0");
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_EQ("{\"deltaPackets\": 88, "
           "\"Engine_uriFull\": [\"1\"], "
           "\"sessionLenClient\": 899, "
           "\"Engine_applicationIdEnd\": 1234, "
           "\"uuid\": \"ABC123\", "
           "\"Engine_refererServer\": [\"notThisOne\"], "
           "\"packetCount\": 88, "
           "\"sessionLenServer\": 567, "
           "\"Engine_server\": [\"thisname12345\"], "
           "\"Engine_domain\": [\"aDomain12345\"], "
           "\"deltaSessionLenServer\": 567, "
           "\"Engine_version\": [\"4.0\"], "
           "\"deltaSessionLenClient\": 899, "
           "\"Engine_applicationEnd\": [\"test\"], "
           "\"Engine_login\": [\"aLogin\"], "
           "\"Engine_method\": [\"RUN\", \"COMMAND\", \"LONGLONGLONGLONG\"], "
           "\"Engine_uri\": [\"not/this/one\"], "
           "\"Engine_sender\": [\"test1_123456\"], "
           "\"Engine_receiver\": [\"test2_123\"], "
           "\"Engine_subject\": [\"test3_12345\"]}", encodedMessage);

}