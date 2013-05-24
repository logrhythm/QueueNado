/*
 * File: ThreadContainerTest.cpp
 * Author: John Gress
 * 
 * Created on May 24, 2013
 */
#include "ThreadContainerTest.h"
#include "ThreadContainer.h"

TEST_F(ThreadContainerTest, testConstructor) {
   ThreadContainer<TestThread*> tc("TestThreads");
}

TEST_F(ThreadContainerTest, testNewDelete) {
   ThreadContainer<TestThread*>* ptc = new ThreadContainer<TestThread*>("TestThreads");
   delete ptc;
}

TEST_F(ThreadContainerTest, testAddThreads) {
   ThreadContainer<TestThread*> tc("TestThreads");
   tc.AddThread(new TestThread(1,0));
   EXPECT_EQ(1, tc.Size());
   tc.Clear();
   EXPECT_EQ(0, tc.Size());
}

TEST_F(ThreadContainerTest, testThreadRunStopZeroThreads) {
   ThreadContainer<TestThread*> tc("TestThreads");

   int ItLoopCount(0);

   EXPECT_EQ(0, tc.Size());

   for ( ThreadContainer<TestThread*>::iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_FALSE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(0, ItLoopCount);

   tc.StartThreads();
   tc.StopThreads();
   tc.Clear();
   EXPECT_EQ(0, tc.Size());
}

TEST_F(ThreadContainerTest, testThreadRunStopOneThread) {
   ThreadContainer<TestThread*> tc("TestThreads");
   tc.AddThread(new TestThread(1,0));
   EXPECT_EQ(1, tc.Size());

   int ItLoopCount(0);
   for ( ThreadContainer<TestThread*>::iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_FALSE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(1, ItLoopCount);

   tc.StartThreads();

   ItLoopCount = 0;
   for ( ThreadContainer<TestThread*>::iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_TRUE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(1, ItLoopCount);

   tc.StopThreads();

   ItLoopCount = 0;
   for ( ThreadContainer<TestThread*>::iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_FALSE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(1, ItLoopCount);

   tc.Clear();
   EXPECT_EQ(0, tc.Size());
}

TEST_F(ThreadContainerTest, testThreadRunStopManyThreads) {
   ThreadContainer<TestThread*> tc("TestThreads");

   unsigned int TotalNumThreads(100);
   for (unsigned int threadNum = 1; threadNum <= TotalNumThreads; threadNum++) {
      tc.AddThread(new TestThread(threadNum,0));
   }
   EXPECT_EQ(TotalNumThreads, tc.Size());

   int ItLoopCount(0);
   for ( ThreadContainer<TestThread*>::const_iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_FALSE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(TotalNumThreads, ItLoopCount);

   tc.StartThreads();

   ItLoopCount = 0;
   for ( ThreadContainer<TestThread*>::const_iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_TRUE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(TotalNumThreads, ItLoopCount);

   tc.StopThreads();

   ItLoopCount = 0;
   for ( ThreadContainer<TestThread*>::const_iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_FALSE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(TotalNumThreads, ItLoopCount);

   tc.Clear();
   EXPECT_EQ(0, tc.Size());
}

TEST_F(ThreadContainerTest, testThreadRunStopThreadDoesNotJoin) {
   ThreadContainer<TestThread*> tc("TestThreads");

   unsigned int TotalNumThreads(3);
   // 1st thread sleeps 10 seconds
   tc.AddThread(new TestThread(1,10));
   // 2nd thread sleeps 20 seconds
   tc.AddThread(new TestThread(2,20));
   // 3rd threads sleeps 45 seconds. 3rd thread does not join so future is not set.
   tc.AddThread(new TestThread(3,40));

   EXPECT_EQ(TotalNumThreads, tc.Size());

   tc.StartThreads();

   int ItLoopCount(0);
   for ( ThreadContainer<TestThread*>::const_iterator it = tc.begin();
         it != tc.end(); it++ ) {
      EXPECT_TRUE(it->first->GetRunState());
      ItLoopCount++;
   }
   EXPECT_EQ(TotalNumThreads, ItLoopCount);

   tc.StopThreads();

   ItLoopCount = 0;
   int ThreadStopCount(0);
   int ThreadStillRunningCount(0);
   for ( ThreadContainer<TestThread*>::const_iterator it = tc.begin();
         it != tc.end(); it++ ) {
      if (it->first->GetRunState()) {
         ThreadStillRunningCount++;
      } else {
         ThreadStopCount++;
      }
      ItLoopCount++;
   }
   EXPECT_EQ(TotalNumThreads, ItLoopCount);
   EXPECT_EQ(2, ThreadStopCount);
   EXPECT_EQ(1, ThreadStillRunningCount);

   tc.Clear();
   EXPECT_EQ(0, tc.Size());
}

