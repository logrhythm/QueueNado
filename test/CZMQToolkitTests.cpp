
#include "CZMQToolkitTests.h"
#include <czmq.h>
#include <zlib.h>


TEST_F(CZMQToolkitTests, HighWaterMark) {
   ASSERT_TRUE(mReplySocket != nullptr);
   ASSERT_TRUE(mRequestSocket != nullptr);

   const int expectedHighWaterMark(15);
   CZMQToolkit::setHWMAndBuffer(mReplySocket, expectedHighWaterMark);
   CZMQToolkit::PrintCurrentHighWater(mReplySocket, mTarget);

   EXPECT_EQ(expectedHighWaterMark, zsocket_rcvhwm(mReplySocket));
   EXPECT_EQ(expectedHighWaterMark, zsocket_sndhwm(mReplySocket));
}

TEST_F(CZMQToolkitTests, SendExistingMessageFailures) {
   ASSERT_TRUE(mReplySocket != nullptr);
   ASSERT_TRUE(mRequestSocket != nullptr);
   zmsg_t* message = nullptr;
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, nullptr));
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, mRequestSocket));
   message = zmsg_new();
   zmsg_addmem(message, "abc", 3);
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, nullptr));
   zmsg_destroy(&message);
}

TEST_F(CZMQToolkitTests, SendExistingMessage) {
   ASSERT_TRUE(mReplySocket != nullptr);
   ASSERT_TRUE(mRequestSocket != nullptr);
   zmsg_t* message = zmsg_new();
   zmsg_addmem(message, "abc", 3);
   ASSERT_TRUE(CZMQToolkit::SendExistingMessage(message, mRequestSocket));
   zmsg_t* gotMessage = zmsg_recv(mReplySocket);
   ASSERT_EQ(1, zmsg_size(gotMessage));
   zframe_t* frame = zmsg_pop(gotMessage);
   ASSERT_EQ(3, zframe_size(frame));
   std::string aString(reinterpret_cast<char*> (zframe_data(frame)), 3);
   ASSERT_EQ("abc", aString);
   zframe_destroy(&frame);
   zmsg_destroy(&gotMessage);
   zmsg_destroy(&message);
}

