#include <stdlib.h>
#include "JSONEncoder.h"
#include "JSONEncoderTest.h"
#include "g2log.hpp"
#include "DpiMsgLR.h"

namespace {
   bool StringContains(const std::string& input, const std::string& pattern) {
      if (input.find(pattern) != std::string::npos) {
         return true;
      } else {
         std::cout << input << " does not contain " << pattern << std::endl;
      }
      return false;
   }
}
TEST_F(JSONEncoderTest, Valgrind) {
   networkMonitor::DpiMsgLR testMsg;
   
   testMsg.set_sessionid("abc123");
   testMsg.set_written(true);
   testMsg.set_portdest(123);
   testMsg.add_application_endq_proto_base("abc");
   testMsg.add_application_endq_proto_base("def");
   testMsg.set_timeupdated(123456789);
   int iterations = 1000;
   while (iterations-- > 0) {
      JSONEncoder encoder(&testMsg);
      std::string encodedMessage = encoder.Encode();
   }
}
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

   EXPECT_TRUE(StringContains(encodedMessage,"{\"sessionId\": \"\"}"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAString) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"{\"sessionId\": \"ABC123\"}"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_portdest(123);
   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"\"portDest\": 123"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithIpSrcDst) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_ipsource(0x11223344);
   testMsg.set_ipdest(0x55667788);
   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"ipDest\": \"136.119.102.85\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"ipSource\": \"68.51.34.17\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt64) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_timedelta(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"timeDelta\": 12345678900"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithATimeT) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_timestart(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"timeStart\": \"2361/03/21 19:15:00\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithEthSrcDst) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_ethsource(12345678900L);
   testMsg.set_ethdest(12345678901L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"ethDest\": \"35:1c:dc:df:02:00\""));   
   EXPECT_TRUE(StringContains(encodedMessage,"\"ethSource\": \"34:1c:dc:df:02:00\""));
   
}

TEST_F(JSONEncoderTest, EncodeAMessageWithRepeatedStringField) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   testMsg.add_accept_encodingq_proto_http("1");
   testMsg.add_accept_encodingq_proto_http("2");
   testMsg.add_accept_encodingq_proto_http("3");

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"#acceptEncoding\": [\"1\", \"2\", \"3\"]"));   

}

TEST_F(JSONEncoderTest, EncodeAMessageWithSingleBool) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   testMsg.set_captured(true);
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"captured\": true"));   
   testMsg.set_captured(false);
   JSONEncoder encoder2(&testMsg);

   encodedMessage = encoder2.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"captured\": false")); 
}

TEST_F(JSONEncoderTest, EncodeAMessageWithRenamer) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   testMsg.add_uri_fullq_proto_http("1");
   testMsg.add_application_endq_proto_base("test");
   testMsg.set_application_id_endq_proto_base(1234);
   testMsg.set_bytesserver(567);
   testMsg.set_bytesserverdelta(567);
   testMsg.set_bytesclient(899);
   testMsg.set_bytesclientdelta(899);
   testMsg.set_packettotal(88);
   testMsg.set_packetsdelta(88);
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
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"#server\": [\"thisname12345\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"packetsDelta\": 88")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#uri\": [\"not/this/one\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesClient\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#refererServer\": [\"notThisOne\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#method\": [\"RUN\", \"COMMAND\", \"LONGLONGLONGLONG\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"packetTotal\": 88")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesServer\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesServerDelta\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesClientDelta\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#subject\": [\"test3_12345\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#applicationEnd\": [\"test\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#applicationIdEnd\": 1234")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#login\": [\"aLogin\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#uriFull\": [\"1\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#version\":"" [\"4.0\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#domain\": [\"aDomain12345\"]")); 
}

TEST_F(JSONEncoderTest, encodesQuickSearchFields) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   testMsg.add_application_endq_proto_base("test");
   testMsg.add_application_endq_proto_base("test2");
   testMsg.add_applicationq_proto_base("test");
   testMsg.add_applicationq_proto_base("test2");
   testMsg.set_bytesserver(567);
   testMsg.set_bytesserverdelta(67);
   testMsg.set_bytesclient(899);
   testMsg.set_bytesclientdelta(99);
   testMsg.set_timeupdated(1000999);
   testMsg.set_timestart(1000000);

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesClient\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application\": [\"test\", \"test2\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesServer\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesServerDelta\": 67")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesClientDelta\": 99")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#applicationEnd\": [\"test\", \"test2\"]")); 

   testMsg.UpdateQuickSearchFields();
   encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"sessionId\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesClient\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application\": [\"test\", \"test2\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesServer\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesServerDelta\": 67")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesClientDelta\": 99")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#applicationEnd\": [\"test\", \"test2\"]"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"applicationPath\": \"/test/test2\"")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"application\": \"test2\"")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesTotal\": 1466"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytesTotalDelta\": 166"));
   
}