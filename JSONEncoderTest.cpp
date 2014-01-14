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
   
   testMsg.set_session_id("abc123");
   testMsg.set_written(true);
   testMsg.set_port_dest(123);
   testMsg.add_application_endq_proto_base("abc");
   testMsg.add_application_endq_proto_base("def");
   testMsg.set_time_updated(123456789);
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

   EXPECT_FALSE(StringContains(encodedMessage,"{\"session_id\": \"\"}"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAString) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"{\"session_id\": \"ABC123\"}"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_port_dest(123);
   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"\"port_dest\": 123"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithIpSrcDst) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_ip_source(0x11223344);
   testMsg.set_ip_dest(0x55667788);
   std::string encodedMessage = encoder.Encode();
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"ip_dest\": \"136.119.102.85\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"ip_source\": \"68.51.34.17\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt64) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_time_delta(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"time_delta\": 12345678900"));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithATimeT) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_time_start(12345678900L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"time_start\": \"2361/03/21 19:15:00\""));
}

TEST_F(JSONEncoderTest, EncodeAMessageWithEthSrcDst) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_mac_source(12345678900L);
   testMsg.set_mac_dest(12345678901L);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"mac_dest\": \"35:1c:dc:df:02:00\""));   
   EXPECT_TRUE(StringContains(encodedMessage,"\"mac_source\": \"34:1c:dc:df:02:00\""));
   
}

TEST_F(JSONEncoderTest, EncodeAMessageWithRepeatedStringField) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   testMsg.add_accept_encodingq_proto_http("1");
   testMsg.add_accept_encodingq_proto_http("2");
   testMsg.add_accept_encodingq_proto_http("3");

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"#accept_encoding\": [\"1\", \"2\", \"3\"]"));   

}

TEST_F(JSONEncoderTest, EncodeAMessageWithSingleBool) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   testMsg.set_captured(true);
   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"captured\": true"));   
   testMsg.set_captured(false);
   JSONEncoder encoder2(&testMsg);

   encodedMessage = encoder2.EncodeWithCallback(testMsg.CleanupName);

   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"captured\": false")); 
}

TEST_F(JSONEncoderTest, EncodeAMessageWithRenamer) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   testMsg.add_uri_fullq_proto_http("1");
   testMsg.add_application_endq_proto_base("test");
   testMsg.set_application_id_endq_proto_base(1234);
   testMsg.set_bytes_dest(567);
   testMsg.set_bytes_dest_delta(567);
   testMsg.set_bytes_source(899);
   testMsg.set_bytes_source_delta(899);
   testMsg.set_packet_total(88);
   testMsg.set_packets_delta(88);
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
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"#server\": [\"thisname12345\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"packets_delta\": 88")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#uri\": [\"not/this/one\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_source\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#referer_server\": [\"notThisOne\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#method\": [\"RUN\", \"COMMAND\", \"LONGLONGLONGLONG\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"packe_total\": 88")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_dest\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_dest_delta\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_source_delta\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#subject\": [\"test3_12345\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_end\": [\"test\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_i_end\": 1234")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#login\": [\"aLogin\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#uri_full\": [\"1\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#version\":"" [\"4.0\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#domain\": [\"aDomain12345\"]")); 
}

TEST_F(JSONEncoderTest, encodesQuickSearchFields) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_session_id("ABC123");
   testMsg.add_application_endq_proto_base("test");
   testMsg.add_application_endq_proto_base("test2");
   testMsg.add_applicationq_proto_base("test");
   testMsg.add_applicationq_proto_base("test2");
   testMsg.set_bytes_dest(567);
   testMsg.set_bytes_dest_delta(67);
   testMsg.set_bytes_source(899);
   testMsg.set_bytes_source_delta(99);
   testMsg.set_time_updated(1000999);
   testMsg.set_time_start(1000000);

   JSONEncoder encoder(&testMsg);

   std::string encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_source\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application\": [\"test\", \"test2\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_dest\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_dest_delta\": 67")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_source_delta\": 99")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_end\": [\"test\", \"test2\"]")); 

   testMsg.UpdateQuickSearchFields();
   encodedMessage = encoder.EncodeWithCallback(testMsg.CleanupName);
   EXPECT_TRUE(StringContains(encodedMessage,"\"session_id\": \"ABC123\""));
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_source\": 899")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application\": [\"test\", \"test2\"]")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_dest\": 567")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_dest_delta\": 67")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_source_delta\": 99")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"#application_end\": [\"test\", \"test2\"]"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"application_path\": \"/test/test2\"")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"application\": \"test2\"")); 
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_total\": 1466"));
   EXPECT_TRUE(StringContains(encodedMessage,"\"bytes_total_delta\": 166"));
   
}
