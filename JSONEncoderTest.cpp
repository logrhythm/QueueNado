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
   
   EXPECT_EQ("{\"uuid\": \"\"}",encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithAString) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   
   std::string encodedMessage = encoder.Encode();
   
   EXPECT_EQ("{\"uuid\": \"ABC123\"}",encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_destport(123);
   std::string encodedMessage = encoder.Encode();
   
   EXPECT_EQ("{\"uuid\": \"ABC123\", \"destport\": 123}",encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithAnInt64) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   JSONEncoder encoder(&testMsg);
   testMsg.set_deltatime(12345678900L);
   
   std::string encodedMessage = encoder.Encode();
   
   EXPECT_EQ("{\"uuid\": \"ABC123\", \"deltatime\": 12345678900}",encodedMessage);
}
TEST_F(JSONEncoderTest, EncodeAMessageWithRepeatedStringField) {
   networkMonitor::DpiMsgLR testMsg;

   testMsg.set_uuid("ABC123");
   testMsg.add_accept_encodingq_proto_http("1");
   testMsg.add_accept_encodingq_proto_http("2");
   testMsg.add_accept_encodingq_proto_http("3");
   
   JSONEncoder encoder(&testMsg);
   
   std::string encodedMessage = encoder.Encode();
   
   EXPECT_EQ("{\"uuid\": \"ABC123\", \"accept_encodingq_proto_http\": [\"1\", \"2\", \"3\"]}",encodedMessage);
}