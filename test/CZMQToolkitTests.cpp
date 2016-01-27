
#include "CZMQToolkitTests.h"
#include <czmq.h>
#include <zlib.h>


TEST_F(CZMQToolkitTests, HighWaterMark) {
   ASSERT_TRUE(mRepSocket != nullptr);
   ASSERT_TRUE(mReqSocket != nullptr);

   const int expectedHighWaterMark(15);
   CZMQToolkit::setHWMAndBuffer(mRepSocket, expectedHighWaterMark);
   CZMQToolkit::PrintCurrentHighWater(mRepSocket, mTarget);

   EXPECT_EQ(expectedHighWaterMark, zsocket_rcvhwm(mRepSocket));
   EXPECT_EQ(expectedHighWaterMark, zsocket_sndhwm(mRepSocket));
}

TEST_F(CZMQToolkitTests, SendExistingMessageFailures) {
   ASSERT_TRUE(mRepSocket != nullptr);
   ASSERT_TRUE(mReqSocket != nullptr);
   zmsg_t* message = nullptr;
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, nullptr));
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, mReqSocket));
   message = zmsg_new();
   zmsg_addmem(message, "abc", 3);
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, nullptr));
   zmsg_destroy(&message);
}

TEST_F(CZMQToolkitTests, SendExistingMessage) {
   ASSERT_TRUE(mRepSocket != nullptr);
   ASSERT_TRUE(mReqSocket != nullptr);
   zmsg_t* message = zmsg_new();
   zmsg_addmem(message, "abc", 3);
   ASSERT_TRUE(CZMQToolkit::SendExistingMessage(message, mReqSocket));
   zmsg_t* gotMessage = zmsg_recv(mRepSocket);
   ASSERT_EQ(1, zmsg_size(gotMessage));
   zframe_t* frame = zmsg_pop(gotMessage);
   ASSERT_EQ(3, zframe_size(frame));
   std::string aString(reinterpret_cast<char*> (zframe_data(frame)), 3);
   ASSERT_EQ("abc", aString);
   zframe_destroy(&frame);
   zmsg_destroy(&gotMessage);
   zmsg_destroy(&message);
}

