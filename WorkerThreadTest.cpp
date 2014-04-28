#include "WorkerThreadTest.h"
#include "WorkerThread.h"
#include <g2log.hpp>
#include <thread>
#include "Death.h"

void ClassWithAWorker::WaitLoop() {
   mRunning = true;
   std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void ClassWithAWorker::WaitLoopWithArgs(unsigned int msSleep) {
   mRunning = true;
   std::this_thread::sleep_for(std::chrono::milliseconds(msSleep));
}


void ClassWithAWorker::LoopThatThrowsInt(const int throwMe) {
   RaiiTrownWasMade flag(mHasThrownAlready);
   std::this_thread::sleep_for(std::chrono::milliseconds(1));

   // skip annoying printouts
   if (!mHasThrownAlready) {
      throw throwMe;
   }    
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
   
   RaiiDeathCleanup deathCleaner;
   Death::SetupExitHandler();
   EXPECT_FALSE(Death::WasKilled());
   {
      WorkerThread testThread(&ClassWithAWorker::LoopThatThrowsInt, &hasWorker, 100);
      size_t counter{0};
      while(hasWorker.mRunning == false && ++counter < 100) {
         if (hasWorker.mHasThrownAlready) {
           testThread.Stop();
         }
         std::this_thread::yield;
      }  
      testThread.Stop();
   }
   EXPECT_TRUE(Death::WasKilled());
   EXPECT_TRUE(Death::Message().find("EXIT trigger caused by broken Contract: CHECK") != std::string::npos);
}

#else 

TEST_F(WorkerThreadTest, nullTest) {
   EXPECT_TRUE(true);
}

#endif
