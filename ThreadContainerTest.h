/*
 * File: ThreadContainerTest.h
 * Author: John Gress
 * 
 * Created on May 24, 2013
 */

#pragma once

#include "gtest/gtest.h"
#include <boost/thread.hpp>

class ThreadContainerTest : public ::testing::Test {
public:

   ThreadContainerTest() {
   };

   ~ThreadContainerTest() {
   }

protected:

private:

};

class TestThread {
   public:
      TestThread( unsigned int threadId, unsigned int shutdownDelay ) :
         mThreadId(threadId),
         mShutdownDelay(shutdownDelay),
         bRun(false) {
         }

      ~TestThread() { }

      unsigned int GetThreadId() {
         return mThreadId;
      }

      bool GetRunState() {
         return bRun;
      }


      boost::thread* Start() {
         mThread = boost::thread(&TestThread::TestMain, this);
         return (&mThread);
      }

      void TestMain() {
         bRun = true;
         try {
            while (1) {
               boost::this_thread::sleep(boost::posix_time::milliseconds(100));
               boost::this_thread::interruption_point();
            }
         } catch (...) {
         }
         boost::this_thread::sleep(boost::posix_time::seconds(mShutdownDelay));
         bRun = false;
      }

   private:
      boost::thread mThread;
      unsigned int mThreadId;
      unsigned int mShutdownDelay;
      bool bRun;
};

