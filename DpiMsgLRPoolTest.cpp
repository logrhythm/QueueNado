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

//DEBUG ONLY TESTS AGAINST PROTECTED METHODS
#ifdef LR_DEBUG

TEST_F(DpiMsgLRPoolTest, OverThreshold) {
   auto pool = new MockDpiMsgLRPool;
   EXPECT_TRUE(pool->OverGivenThreshold(101, 100, 0.01, 100, 1000));
   EXPECT_FALSE(pool->OverGivenThreshold(100, 100, 0.01, 100, 1000));
   EXPECT_TRUE(pool->OverGivenThreshold(1002, 100200, 0.01, 100, 1000));
   EXPECT_FALSE(pool->OverGivenThreshold(1001, 100100, 0.01, 100, 1002));
   EXPECT_TRUE(pool->OverGivenThreshold(101, 0, 0.01, 100, 1000));
   EXPECT_FALSE(pool->OverGivenThreshold(0, 0, 0.01, 100, 1000));
   delete pool;

}

TEST_F(DpiMsgLRPoolTest, ReportSize) {
    MockDpiMsgLRPool pool;
    DpiMsgLRPool::FreePoolOfMessages fPool;
    DpiMsgLRPool::UsedPoolOfMessages uPool;
    EXPECT_FALSE(pool.ReportSize(1, fPool, uPool));
    std::this_thread::sleep_for(std::chrono::seconds(6));
    EXPECT_TRUE(pool.ReportSize(1, fPool, uPool));
}

TEST_F(DpiMsgLRPoolTest, GetStatsSender) {
   auto pool = new MockDpiMsgLRPool;
   auto a = pool->GetStatsSender(1);
   auto b = pool->GetStatsSender(1);
   ASSERT_EQ(a, b);
   auto c = pool->GetStatsSender(2);
   ASSERT_NE(a, c);
   delete pool;
}

TEST_F(DpiMsgLRPoolTest, GetStatsTimers) {
   auto pool = new MockDpiMsgLRPool;
   auto a = pool->GetStatsTimer(1);
   auto b = pool->GetStatsTimer(1);
   ASSERT_EQ(a, b);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   auto c = pool->GetStatsTimer(2);
   ASSERT_NE(a, c);
   delete pool;

}

TEST_F(DpiMsgLRPoolTest, SetStatsTimers) {
   auto pool = new MockDpiMsgLRPool;
   auto a = pool->GetStatsTimer(1);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   auto time = std::time(NULL);
   pool->SetStatsTimer(1, time);
   a = pool->GetStatsTimer(1);
   ASSERT_EQ(time, a);
   delete pool;
}

TEST_F(DpiMsgLRPoolTest, SetStatsTimersThatDoesntExist) {
   auto pool = new MockDpiMsgLRPool;
   auto time = std::time(NULL);
   pool->SetStatsTimer(1, time);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   auto a = pool->GetStatsTimer(1);
   //we should always get something before it's ever set.
   ASSERT_NE(time, a);
   delete pool;
}

#endif

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
#ifdef LR_DEBUG

TEST_F(DpiMsgLRPoolTest, DpiMsgSize) {

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
}
#endif


