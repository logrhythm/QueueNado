#pragma once
#include "gtest/gtest.h"
#include "WorkerThread.h"
#include "include/global.h"
#include "gmock/gmock.h"
#include <atomic>
class WorkerThreadTest : public ::testing::Test {
public:
   WorkerThreadTest() {   };
   void WaitLoop();
protected:
   virtual void SetUp() {   };
   virtual void TearDown() {   };
private:
};

class ClassWithAWorker {
public:
   ClassWithAWorker() : mRunning(false), mRun(false){}
   ~ClassWithAWorker() { EXPECT_FALSE(mRunning); }
   void WaitLoop();
   void WaitLoopWithArgs(unsigned int msSleep);
   void LoopThatThrowsInt(const int throwMe); 
   std::string What();
   bool mRunning;
   std::atomic<bool> mRun;
};