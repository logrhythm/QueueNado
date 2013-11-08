#include "WorkerThreadTest.h"
#include "WorkerThread.h"
#include <thread>

void ClassWithAWorker::WaitLoop() {
   while (mRun) {
      mRunning = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
}
#ifdef LR_DEBUG

TEST_F(WorkerThreadTest, RIAAWorks) {
   ClassWithAWorker hasWorker;
   {
      WorkerThread testThread(hasWorker.mRun,&ClassWithAWorker::WaitLoop, std::ref(hasWorker));
   }
   hasWorker.mRunning = false;
   std::this_thread::sleep_for(std::chrono::milliseconds(2));
   EXPECT_FALSE(hasWorker.mRunning);
   {
      WorkerThread testThread(hasWorker.mRun,&ClassWithAWorker::WaitLoop, std::ref(hasWorker));
      testThread.Stop();
   }
   hasWorker.mRunning = false;
   std::this_thread::sleep_for(std::chrono::milliseconds(2));
   EXPECT_FALSE(hasWorker.mRunning);
}

#else 

TEST_F(WorkerThreadTest, nullTest) {
   EXPECT_TRUE(true);
}

#endif