#include "CZMQToolkitTests.h"

TEST_F(CZMQToolkitTests, CalculateHeadersEmptyMessage) {
   CZMQToolkit::GetHash(&t_data, t_header);
   uLong crc = crc32(0L, NULL, 0);
   EXPECT_EQ(crc, t_header);
}

TEST_F(CZMQToolkitTests, CalculateHeadersShortMessage) {
   MakeSmallData();
   t_data.assign(&t_smallData[0], &t_smallData[0] + 16);
   CZMQToolkit::GetHash(&t_data, t_header);
   uLong crc = crc32(0L, NULL, 0);
   crc = crc32(crc, reinterpret_cast<unsigned char*> (&t_smallData[0]), 16);
   EXPECT_EQ(crc, t_header);
}

TEST_F(CZMQToolkitTests, CalculateHeadersBigMessage) {
   MakeBigData(1500);
   t_data.assign(t_bigData, t_bigData + t_bigDataSize);
   CZMQToolkit::GetHash(&t_data, t_header);
   uLong crc = crc32(0L, NULL, 0);
   crc = crc32(crc, &t_bigData[0], t_bigDataSize);
   EXPECT_EQ(crc, t_header);
}

TEST_F(CZMQToolkitTests, CalculateHeadersBigMessageBench) {

   MakePacketsData();
   t_data.assign(t_bigData, t_bigData + t_bigDataSize);
   int iterations = PACKETS_TO_TEST >> 2;
   SetExpectedTime(iterations, t_bigDataSize, 5000, 500000L);
   StartTimedSection();
   while (iterations-- >= 0) {
      CZMQToolkit::GetHash(&t_data, t_header);
   }
   EndTimedSection();
   if (PACKETS_TO_TEST > 1000) {
      EXPECT_TRUE(TimedSectionPassed());
   }
}

TEST_F(CZMQToolkitTests, CalculateHeadersSmallMessageBench) {

   MakeSmallData();
   t_data.assign(t_smallData, t_smallData + 16);
   int iterations = PACKETS_TO_TEST << 4;
   SetExpectedTime(iterations, 16, 5000, 4000000L);
   StartTimedSection();
   while (iterations-- >= 0) {
      CZMQToolkit::GetHash(&t_data, t_header);
   }
   EndTimedSection();
   if (PACKETS_TO_TEST > 1000) {
      EXPECT_TRUE(TimedSectionPassed());
   }

}

TEST_F(CZMQToolkitTests, ValidateMessage) {

   zmsg_t* message = NULL;
   ASSERT_FALSE(CZMQToolkit::IsValidMessage(message));
   message = zmsg_new();
   ASSERT_TRUE(CZMQToolkit::IsValidMessage(message));
   for (int i = 0; i < ZMQ_KILL_MESSAGE; i++) {
      zmsg_addmem(message, "", 0);
   }
   ASSERT_FALSE(CZMQToolkit::IsValidMessage(message));
}

TEST_F(CZMQToolkitTests, SendShutdownMessage) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   CZMQToolkit::SendShutdownMessage(NULL);
   CZMQToolkit::SendShutdownMessage(mReqSocket);
   zmsg_t* message = zmsg_recv(mRepSocket);
   ASSERT_FALSE(message == NULL);
   ASSERT_FALSE(CZMQToolkit::IsValidMessage(message));
}

TEST_F(CZMQToolkitTests, SendAndGetSizeTFromSocketFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   size_t testVal = 1023456;
   ASSERT_FALSE(CZMQToolkit::SendSizeTToSocket(NULL, testVal));
   ASSERT_FALSE(CZMQToolkit::GetSizeTFromSocket(NULL, testVal));

   zctx_destroy(&mContext);
   ASSERT_FALSE(CZMQToolkit::SendSizeTToSocket(mReqSocket, testVal));
   ASSERT_FALSE(CZMQToolkit::GetSizeTFromSocket(mRepSocket, testVal));
}

TEST_F(CZMQToolkitTests, SendAndGetSizeTFromSocket) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   size_t testVal = 1023456;
   size_t replyVal;

   ASSERT_TRUE(CZMQToolkit::SendSizeTToSocket(mReqSocket, testVal));
   ASSERT_TRUE(CZMQToolkit::GetSizeTFromSocket(mRepSocket, replyVal));
   ASSERT_EQ(testVal, replyVal);
}
// TODO, this test causes an assertion in the new czmq, but I don't know why
//TEST_F(CZMQToolkitTests, SendAndGetSizeTFromSocketInvalid) {
//   ASSERT_TRUE(mRepSocket != NULL);
//   ASSERT_TRUE(mReqSocket != NULL);
//   size_t replyVal;
//
//   ASSERT_TRUE(CZMQToolkit::SendBlankMessage(mReqSocket));
//   ASSERT_FALSE(CZMQToolkit::GetSizeTFromSocket(mRepSocket, replyVal));
//}
TEST_F(CZMQToolkitTests, GetSizeTFromSocketShutdownMsg) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);
   size_t replyVal;
   CZMQToolkit::SendShutdownMessage(mReqSocket);
   ASSERT_FALSE(CZMQToolkit::GetSizeTFromSocket(mRepSocket, replyVal));

}

TEST_F(CZMQToolkitTests, SendBlankMessageFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   size_t testVal = 1023456;
   ASSERT_FALSE(CZMQToolkit::SendBlankMessage(NULL));
   ASSERT_FALSE(CZMQToolkit::PopAndDiscardMessage(NULL));

   zctx_destroy(&mContext);
   ASSERT_FALSE(CZMQToolkit::SendBlankMessage(mReqSocket));
   ASSERT_FALSE(CZMQToolkit::PopAndDiscardMessage(mRepSocket));
}

TEST_F(CZMQToolkitTests, SendBlankMessage) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);

   ASSERT_TRUE(CZMQToolkit::SendBlankMessage(mReqSocket));
   ASSERT_TRUE(CZMQToolkit::PopAndDiscardMessage(mRepSocket));
}

TEST_F(CZMQToolkitTests, PopAndDiscardMessageShutdownMsg) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   CZMQToolkit::SendShutdownMessage(mReqSocket);
   ASSERT_FALSE(CZMQToolkit::PopAndDiscardMessage(mRepSocket));

}

TEST_F(CZMQToolkitTests, SocketFIFOFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_FALSE(CZMQToolkit::SocketFIFO(NULL));
   zctx_destroy(&mContext);
   ASSERT_FALSE(CZMQToolkit::SocketFIFO(mRepSocket));
}

TEST_F(CZMQToolkitTests, SocketFIFO) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);

   size_t testVal = 1023456;
   size_t replyVal;

   ASSERT_TRUE(CZMQToolkit::SendSizeTToSocket(mReqSocket, testVal));
   ASSERT_TRUE(CZMQToolkit::SocketFIFO(mRepSocket));
   ASSERT_TRUE(CZMQToolkit::GetSizeTFromSocket(mReqSocket, replyVal));
   ASSERT_EQ(testVal, replyVal);
}
TEST_F(CZMQToolkitTests, SocketFIFOInvalidLength) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);

   zmsg_t* bullet = zmsg_new();
   zmsg_addmem(bullet,"",0);
   zmsg_addmem(bullet,"",0);
   ASSERT_TRUE(CZMQToolkit::SendExistingMessage(bullet,mReqSocket));
   ASSERT_FALSE(CZMQToolkit::SocketFIFO(mRepSocket));
}
TEST_F(CZMQToolkitTests, SocketFIFOShutdownMsg) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   CZMQToolkit::SendShutdownMessage(mReqSocket);
   ASSERT_FALSE(CZMQToolkit::SocketFIFO(mRepSocket));

}

TEST_F(CZMQToolkitTests, SendExistingMessageFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   zmsg_t* message = NULL;
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, NULL));
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, mReqSocket));
   message = zmsg_new();
   zmsg_addmem(message, "abc", 3);
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, NULL));
   zctx_destroy(&mContext);
   zclock_sleep(100);
   ASSERT_FALSE(CZMQToolkit::SendExistingMessage(message, mReqSocket));
   zmsg_destroy(&message);
}

TEST_F(CZMQToolkitTests, SendExistingMessage) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
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
   zmsg_destroy(&message);
}

TEST_F(CZMQToolkitTests, PassMessageAlongFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_FALSE(CZMQToolkit::PassMessageAlong(NULL, NULL));
   ASSERT_FALSE(CZMQToolkit::PassMessageAlong(mRepSocket, NULL));
   ASSERT_FALSE(CZMQToolkit::PassMessageAlong(NULL, mReqSocket));
   zctx_destroy(&mContext);
   ASSERT_FALSE(CZMQToolkit::PassMessageAlong(mRepSocket, mReqSocket));
}

TEST_F(CZMQToolkitTests, PassMessageAlong) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);

   size_t testVal = 1023456;
   size_t replyVal;

   ASSERT_TRUE(CZMQToolkit::SendSizeTToSocket(mReqSocket, testVal));
   ASSERT_TRUE(CZMQToolkit::PassMessageAlong(mRepSocket, mRepSocket));
   ASSERT_TRUE(CZMQToolkit::GetSizeTFromSocket(mReqSocket, replyVal));
   ASSERT_EQ(testVal, replyVal);
}

TEST_F(CZMQToolkitTests, PassMessageAlongShutdownMsg) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   CZMQToolkit::SendShutdownMessage(mReqSocket);
   ASSERT_FALSE(CZMQToolkit::PassMessageAlong(mRepSocket, mRepSocket));

}

TEST_F(CZMQToolkitTests, SendStringContentsToSocketFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_FALSE(CZMQToolkit::SendStringContentsToSocket(NULL, NULL));
   ASSERT_FALSE(CZMQToolkit::SendStringContentsToSocket(mRepSocket, NULL));
   ASSERT_FALSE(CZMQToolkit::SendStringContentsToSocket(NULL, mReqSocket));
   zmsg_t* message = NULL;
   ASSERT_TRUE(CZMQToolkit::GetStringFromMessage(message) == NULL);

   zctx_destroy(&mContext);
   ASSERT_FALSE(CZMQToolkit::SendStringContentsToSocket(mRepSocket, mReqSocket));
}

TEST_F(CZMQToolkitTests, SendStringContentsToSocket) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);

   zmsg_t* message = zmsg_new();
   ASSERT_TRUE(CZMQToolkit::GetStringFromMessage(message) == NULL);
   std::string expectedString("abc");
   std::string* aString = new std::string(expectedString);
   zmsg_addmem(message, &(aString), sizeof (std::string*));
   ASSERT_EQ(0, zmsg_send(&message, mReqSocket));
   ASSERT_TRUE(CZMQToolkit::SendStringContentsToSocket(mRepSocket, mRepSocket));
   zmsg_t* reply = zmsg_recv(mReqSocket);
   ASSERT_FALSE(reply == NULL);
   ASSERT_EQ(2, zmsg_size(reply));
   zframe_t* frame = zmsg_pop(reply);
   zframe_destroy(&frame);
   std::string* aReply = CZMQToolkit::GetStringFromMessage(reply);
   ASSERT_FALSE(aReply == NULL);
   ASSERT_EQ(expectedString, *aReply);
   zmsg_destroy(&reply);
   delete aReply;
   delete aString;
}

TEST_F(CZMQToolkitTests, SendStringContentsToSocketShutdownMsg) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   CZMQToolkit::SendShutdownMessage(mReqSocket);
   ASSERT_FALSE(CZMQToolkit::SendStringContentsToSocket(mRepSocket, mRepSocket));

}

TEST_F(CZMQToolkitTests, ForkPartsOfMessageTwoDirectionsFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(NULL, NULL, NULL));
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(mRepSocket, NULL, NULL));
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(NULL, mReqSocket, NULL));
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(NULL, NULL, mReqSocket2));
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(mRepSocket, mReqSocket, NULL));
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(mRepSocket, NULL, mReqSocket2));
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(NULL, mReqSocket, mReqSocket2));

   zctx_destroy(&mContext);
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(mRepSocket, mReqSocket, mReqSocket2));
}

TEST_F(CZMQToolkitTests, ForkPartsOfMessageTwoDirections) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   zmsg_t* message = zmsg_new();
   zmsg_addmem(message, "123", 3);
   zmsg_addmem(message, "4567", 4);
   zmsg_send(&message, mReqSocket);

   ASSERT_TRUE(CZMQToolkit::ForkPartsOfMessageTwoDirections(mRepSocket, mRepSocket, mReqSocket2));
   message = zmsg_recv(mReqSocket);
   ASSERT_EQ(1, zmsg_size(message));
   zframe_t* frame = zmsg_last(message);
   ASSERT_EQ(3, zframe_size(frame));
   std::string aString(reinterpret_cast<char*> (zframe_data(frame)), zframe_size(frame));
   ASSERT_EQ("123", aString);
   zmsg_destroy(&message);
   message = zmsg_recv(mRepSocket);
   ASSERT_EQ(1, zmsg_size(message));
   frame = zmsg_last(message);
   ASSERT_EQ(4, zframe_size(frame));
   aString.assign(reinterpret_cast<char*> (zframe_data(frame)), zframe_size(frame));
   ASSERT_EQ("4567", aString);
   zmsg_destroy(&message);
}

TEST_F(CZMQToolkitTests, ForkPartsOfMessageTwoDirectionsShutdownMsg) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   CZMQToolkit::SendShutdownMessage(mReqSocket);
   ASSERT_FALSE(CZMQToolkit::ForkPartsOfMessageTwoDirections(mRepSocket, mRepSocket, mReqSocket2));

}

TEST_F(CZMQToolkitTests, SendStringWithHashFailures) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   zmsg_t* message = NULL;
   std::string* aString = NULL;
   ASSERT_FALSE(CZMQToolkit::SendStringWithHash(mReqSocket, message, aString));
   message = zmsg_new();
   ASSERT_FALSE(CZMQToolkit::SendStringWithHash(mReqSocket, message, aString));
   zmsg_destroy(&message);
   std::string expectedString("abc");
   aString = new std::string(expectedString);
   ASSERT_FALSE(CZMQToolkit::SendStringWithHash(mReqSocket, message, aString));
   message = zmsg_new();
   ASSERT_FALSE(CZMQToolkit::SendStringWithHash(NULL, message, aString));

   zctx_destroy(&mContext);
   ASSERT_FALSE(CZMQToolkit::SendStringWithHash(mReqSocket, message, aString));
   
   zmsg_destroy(&message);
   delete aString;
}

TEST_F(CZMQToolkitTests, SendStringWithHash) {
   ASSERT_TRUE(mRepSocket != NULL);
   ASSERT_TRUE(mReqSocket != NULL);
   ASSERT_TRUE(mReqSocket2 != NULL);

   zmsg_t* message = zmsg_new();
   std::string expectedString("abc");
   std::string* aString = new std::string(expectedString);

   ASSERT_TRUE(CZMQToolkit::SendStringWithHash(mReqSocket, message, aString));
   ASSERT_TRUE(CZMQToolkit::ForkPartsOfMessageTwoDirections(mRepSocket, mRepSocket, mReqSocket2));
   message = zmsg_recv(mReqSocket);
   ASSERT_EQ(1, zmsg_size(message));
   zframe_t* frame = zmsg_last(message);
   ASSERT_EQ(sizeof (uLong), zframe_size(frame));
   uLong hash = *(reinterpret_cast<uLong*> (zframe_data(frame)));
   ASSERT_EQ(891568578, hash);
   zmsg_destroy(&message);
   message = zmsg_recv(mRepSocket);
   ASSERT_EQ(1, zmsg_size(message));
   frame = zmsg_last(message);
   ASSERT_EQ(sizeof (std::string*), zframe_size(frame));
   ASSERT_EQ(aString, *(reinterpret_cast<std::string**> (zframe_data(frame))));
   zmsg_destroy(&message);
   delete aString;
}