#include "WorkerThreadTest.h"
#include "WorkerThread.h"
#include <thread>

void ClassWithAWorker::WaitLoop() {
   mRunning = true;
   std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void ClassWithAWorker::WaitLoopWithArgs(unsigned int msSleep) {
   mRunning = true;
   std::this_thread::sleep_for(std::chrono::milliseconds(msSleep));
}

void ClassWithAWorker::LoopThatThrowsInt(const int throwMe) {
   throw throwMe;
}

std::string ClassWithAWorker::What() {
   return "TestClass";
}
#ifdef LR_DEBUG

TEST_F(WorkerThreadTest, RIAAWorks) {
   ClassWithAWorker hasWorker;
   {
      WorkerThread testThread(&ClassWithAWorker::WaitLoop, &hasWorker);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
   }
   EXPECT_TRUE(hasWorker.mRunning);
   hasWorker.mRunning = false;
   std::this_thread::sleep_for(std::chrono::milliseconds(2));
   EXPECT_FALSE(hasWorker.mRunning);
   {
      WorkerThread testThread(&ClassWithAWorker::WaitLoop, &hasWorker);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      testThread.Stop();
   }
   EXPECT_TRUE(hasWorker.mRunning);
   hasWorker.mRunning = false;
   std::this_thread::sleep_for(std::chrono::milliseconds(2));
   EXPECT_FALSE(hasWorker.mRunning);
   {
      WorkerThread testThread(&ClassWithAWorker::WaitLoopWithArgs, &hasWorker, 100);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      testThread.Stop();
   }
   EXPECT_TRUE(hasWorker.mRunning);
   hasWorker.mRunning = false;
   std::this_thread::sleep_for(std::chrono::milliseconds(2));
   EXPECT_FALSE(hasWorker.mRunning);
   ::testing::FLAGS_gtest_death_test_style = "threadsafe";
   ASSERT_DEATH({

      WorkerThread testThread(&ClassWithAWorker::LoopThatThrowsInt, &hasWorker, 100);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      testThread.Stop();

   },"EXIT trigger caused by broken Contract: CHECK");

}

#else 

TEST_F(WorkerThreadTest, nullTest) {
   EXPECT_TRUE(true);
}

#endif