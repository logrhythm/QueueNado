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

   EXPECT_FALSE(StringContains(encodedMessage,"{\"SessionID\": \"\"}"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAString) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"{\"SessionID\": \"ABC123\"}"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_portdest(123);
   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"\"PortDest\": 123"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithIpSrcDst) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_ipsource(0x11223344);
   testMsg.set_ipdest(0x55667788);
   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"IPDest\": \"136.119.102.85\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"IPSource\": \"68.51.34.17\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt64) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_timedelta(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"TimeDelta\": 12345678900"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithATimeT) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_timestart(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"TimeStart\": \"2361/03/21 19:15:00\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithEthSrcDst) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_macsource(12345678900L);
   testMsg.set_macdest(12345678901L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"MacDest\": \"35:1c:dc:df:02:00\""));   
   EXPECT_TRUE(StringContains(encodedMessage,"\"MacSource\": \"34:1c:dc:df:02:00\""));
   
}

TEST_F(JSONEncoderTest, EncodeAMessageWithRepeatedStringField) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   testMsg.add_accept_encodingq_proto_http("1");
   testMsg.add_accept_encodingq_proto_http("2");
   testMsg.add_accept_encodingq_proto_http("3");

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"#accept_encoding\": [\"1\", \"2\", \"3\"]"));   

}

TEST_F(JSONEncoderTest, EncodeAMessageWithSingleBool) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   testMsg.set_captured(true);
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"Captured\": true"));   
   testMsg.set_captured(false);
   JSONEncoder encoder2(&testMsg);

   encodedMessage = encoder2.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"Captured\": false")); 
}

TEST_F(JSONEncoderTest, EncodeAMessageWithRenamer) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_sessionid("ABC123");
   testMsg.add_uri_fullq_proto_http("1");
   testMsg.add_application_endq_proto_base("test");
   testMsg.set_application_id_endq_proto_base(1234);
   testMsg.set_bytesdest(567);
   testMsg.set_bytesdestdelta(567);
   testMsg.set_bytessource(899);
   testMsg.set_bytessourcedelta(899);
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
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"#server\": [\"thisname12345\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"PacketsDelta\": 88")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#uri\": [\"not/this/one\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesSource\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#referer_server\": [\"notThisOne\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#method\": [\"RUN\", \"COMMAND\", \"LONGLONGLONGLONG\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"PacketTotal\": 88")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesDest\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesDestDelta\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesSourceDelta\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#subject\": [\"test3_12345\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_end\": [\"test\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_id_end\": 1234")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#login\": [\"aLogin\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#uri_full\": [\"1\"]")); 
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
   testMsg.set_bytesdest(567);
   testMsg.set_bytesdestdelta(67);
   testMsg.set_bytessource(899);
   testMsg.set_bytessourcedelta(99);
   testMsg.set_timeupdated(1000999);
   testMsg.set_timestart(1000000);

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesSource\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application\": [\"test\", \"test2\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesDest\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesDestDelta\": 67")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesSourceDelta\": 99")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_end\": [\"test\", \"test2\"]")); 

   testMsg.UpdateQuickSearchFields();
   encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"SessionID\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesSource\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application\": [\"test\", \"test2\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesDest\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesDestDelta\": 67")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesSourceDelta\": 99")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_end\": [\"test\", \"test2\"]"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"ApplicationPath\": \"/test/test2\"")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"application\": \"test2\"")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesTotal\": 1466"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"BytesTotalDelta\": 166"));
   
}
