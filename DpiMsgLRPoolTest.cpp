#include "DpiMsgLRPoolTest.h"
#include "MockDpiMsgLRPool.h"
#include <thread>
#include <stdlib.h>
#include <time.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "ConfSlave.h"

void SimulatorThread(DpiMsgLRPool* testPool, const int iterations) {
   std::vector<networkMonitor::DpiMsgLR*> messages;

   for (int i = 0; i < iterations; i++) {
      std::vector<networkMonitor::DpiMsgLR*> temp;
      messages.push_back(testPool->GetDpiMsg());
      for (auto it = messages.begin(); it != messages.end(); it++) {

         if (rand() % 0x2 == 0) {
            ASSERT_TRUE(testPool->ReturnDpiMsg(&(*it)));
         } else {
            temp.push_back(*it);
         }
      }
      messages = std::move(temp);
   }
   for (auto jt = messages.begin(); jt != messages.end(); jt++) {
      ASSERT_TRUE(testPool->ReturnDpiMsg(&(*jt)));
   }
}

TEST_F(DpiMsgLRPoolTest, Construction) {
   DpiMsgLRPool* pTestObject = new DpiMsgLRPool;
   delete pTestObject;
}

TEST_F(DpiMsgLRPoolTest, GetAMessageAndReturnIt) {
   DpiMsgLRPool testPool;

   networkMonitor::DpiMsgLR* message1 = testPool.GetDpiMsg();

   EXPECT_FALSE(message1 == NULL);

   EXPECT_TRUE(testPool.ReturnDpiMsg(&message1));

   EXPECT_TRUE(message1 == NULL);

   EXPECT_FALSE(testPool.ReturnDpiMsg(NULL));

   networkMonitor::DpiMsgLR* notMine = new networkMonitor::DpiMsgLR;

   EXPECT_FALSE(testPool.ReturnDpiMsg(&notMine));
   delete notMine;
}

TEST_F(DpiMsgLRPoolTest, HammerTime) {
   std::vector<std::thread*> threads;
   DpiMsgLRPool testPool;
#ifdef LR_DEBUG
   int iterations = 10;
#else
   int iterations = 1000;
#endif
   for (int i = 0; i < iterations; i++) {
      threads.push_back(new std::thread(SimulatorThread, &testPool, rand() % 1000));
   }
   for (auto jt = threads.begin(); jt != threads.end(); jt++) {
      (*jt)->join();
      delete *jt;
   }
}

TEST_F(DpiMsgLRPoolTest, DpiMsgSize) {
#ifdef LR_DEBUG
   MockDpiMsgLRPool testPool;
   Conf conf = networkMonitor::ConfSlave::Instance().GetConf();
   const auto threshold = conf.GetDpiRecycleTheshold();

   networkMonitor::DpiMsgLR* testMsg = new networkMonitor::DpiMsgLR;
   static std::string oneHundredByteString = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";

   while (testMsg->SpaceUsed() < threshold - 100 * 100) {
      for (int i = 0; i < 100; i++) {
         testMsg->add_accept_encodingq_proto_http(oneHundredByteString);
      }
   }
   EXPECT_FALSE(testPool.DpiMsgTooBig(testMsg, threshold));
   for (int i = 0; i < 100; i++) {
      testMsg->add_accept_encodingq_proto_http(oneHundredByteString);
   }
   EXPECT_TRUE(testPool.DpiMsgTooBig(testMsg, threshold));
   testMsg->ClearAll();
   EXPECT_FALSE(testPool.DpiMsgTooBig(testMsg, threshold));
   size_t currentSize = testMsg->SpaceUsed();
   EXPECT_FALSE(testPool.DpiMsgTooBig(testMsg, currentSize + 1));
   testMsg->add_account_uidq_proto_vkontakte(oneHundredByteString);
   EXPECT_TRUE(testPool.DpiMsgTooBig(testMsg, currentSize + 1));
   testMsg->ClearAll();
   EXPECT_TRUE(testPool.DpiMsgTooBig(testMsg, currentSize + 1));
   delete testMsg;
#endif
}


