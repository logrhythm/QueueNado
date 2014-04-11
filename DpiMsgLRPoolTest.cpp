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
      messages.push_back(testPool->GetDpiMsg());
   }
   for (auto jt = messages.begin(); jt != messages.end(); jt++) {
      ASSERT_TRUE(testPool->ReturnDpiMsg(&(*jt)));
   }
   for (auto jt = messages.begin(); jt != messages.end(); jt++) {
      ASSERT_FALSE(testPool->ReturnDpiMsg(&(*jt)));
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
   EXPECT_FALSE(pool->OverGivenThreshold(0, 0, 0, 100, 1000));
   EXPECT_TRUE(pool->OverGivenThreshold(101, 0, -1, 100, 1000));
   EXPECT_FALSE(pool->OverGivenThreshold(99, 0, -1, 100, 1000));
   EXPECT_TRUE(pool->OverGivenThreshold(101, 0, 1.1, 100, 1000));
   EXPECT_TRUE(pool->OverGivenThreshold(101, 100, 1.0, 100, 1000));
   EXPECT_FALSE(pool->OverGivenThreshold(99, 100, 1.0, 100, 1000));
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
   ASSERT_EQ(&pool->GetStatsTimer(1), &pool->GetStatsTimer(1));
   std::this_thread::sleep_for(std::chrono::seconds(1));
   ASSERT_NE(&pool->GetStatsTimer(1), &pool->GetStatsTimer(2));
   delete pool;

}

TEST_F(DpiMsgLRPoolTest, SetStatsTimers) {
   auto pool = new MockDpiMsgLRPool;
   pool->GetStatsTimer(1);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   ASSERT_TRUE(pool->GetStatsTimer(1).ElapsedSec() >= 1);
   pool->SetStatsTimer(1);
   ASSERT_FALSE(pool->GetStatsTimer(1).ElapsedSec() >= 1);
   delete pool;
}

TEST_F(DpiMsgLRPoolTest, SetStatsTimersThatDoesntExist) {
   auto pool = new MockDpiMsgLRPool;
   //we should always get something before it's ever set.
   ASSERT_TRUE(pool->GetStatsTimer(1).ElapsedSec() <= 1);
   delete pool;
}

#endif

TEST_F(DpiMsgLRPoolTest, Construction) {
   DpiMsgLRPool& pTestObject = DpiMsgLRPool::Instance();
   pTestObject;
}

TEST_F(DpiMsgLRPoolTest, GetAMessageAndReturnIt) {
   DpiMsgLRPool& testPool = DpiMsgLRPool::Instance();

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
   DpiMsgLRPool& testPool = DpiMsgLRPool::Instance();
#ifdef LR_DEBUG
   int threadCount = 500;
   int retries = 1;
#else
   int threadCount = 500;
   int retries = 5;
#endif
   int tries = 1;
   while (tries <= retries) {
      for (int i = 0; i < threadCount; i++) {
         threads.push_back(new std::thread(SimulatorThread, &testPool, 200));
      }
      for (auto jt = threads.begin(); jt != threads.end(); jt++) {
         (*jt)->join();
         delete *jt;
      }
      threads.clear();
      std::cout << tries << " Itteration(s) complete" << std::endl;
      ++tries;
   }
}

TEST_F(DpiMsgLRPoolTest, ValidateAllMessagesInAThread) {
   DpiMsgLRPool& testPool = DpiMsgLRPool::Instance();
#ifdef LR_DEBUG
   //   int messageCount = 5000;
   //   int retries = 5;
   int messageCount = 2000;
   int retries = 1;
#else
   int messageCount = 40000;
   int retries = 20;
#endif
   int tries = 1;
   std::vector<networkMonitor::DpiMsgLR*> messages;
   while (tries <= retries) {

      for (int i = 0; i < messageCount; i++) {
         messages.push_back(testPool.GetDpiMsg());
      }
      for (auto jt = messages.begin(); jt != messages.end(); jt++) {
         ASSERT_TRUE(testPool.ReturnDpiMsg(&(*jt)));
      }
      for (auto jt = messages.begin(); jt != messages.end(); jt++) {
         ASSERT_FALSE(testPool.ReturnDpiMsg(&(*jt)));
      }
      messages.clear();
      std::cout << tries << " Itteration(s) complete" << std::endl;
      ++tries;
   }

}
#ifdef LR_DEBUG

TEST_F(DpiMsgLRPoolTest, DpiMsgSize) {

   MockDpiMsgLRPool testPool;
   Conf conf =ConfSlave::Instance().GetConf();
   const auto threshold = conf.GetDpiRecycleTheshold();
   size_t msgBaseSize;
   networkMonitor::DpiMsgLR* testMsg = new networkMonitor::DpiMsgLR;
   msgBaseSize = testMsg->SpaceUsed();
   static std::string oneHundredByteString = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012";

   while (testMsg->SpaceUsed() < (threshold - 100 * 100 )) {
      for (int i = 0; i < 100 && testMsg->SpaceUsed() < (threshold - 100 * 100 ); i++) {
         testMsg->add_accept_encodingq_proto_http(oneHundredByteString);
      }
      
   }
   EXPECT_FALSE(testPool.DpiMsgTooBig(testMsg, threshold));
   for (int i = 0; i < 150; i++) {
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