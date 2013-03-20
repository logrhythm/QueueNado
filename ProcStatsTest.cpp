#include "ProcStatsTest.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 
#include <sys/syscall.h> 

TEST_F(ProcStatsTest, ConstructSingleton) {
   if (geteuid() == 0) {
      EXPECT_EQ(0, mProcStats.GetUserPercent());
      EXPECT_EQ(0, mProcStats.GetIOWaitPercent());
      EXPECT_EQ(0, mProcStats.GetSystemPercent());
   }
}

TEST_F(ProcStatsTest, UpdateDoesSomething) {

   boost::this_thread::sleep(boost::posix_time::seconds(1));
   mProcStats.Update();
   EXPECT_NE(0, mProcStats.GetUserPercent());
   //EXPECT_NE(0, mProcStats.GetIOWaitPercent());
   EXPECT_NE(0, mProcStats.GetSystemPercent());
}

TEST_F(ProcStatsTest, ThreadRegistration) {
   if (geteuid() == 0) {
      std::string threadName("TestThread");
      mProcStats.ThreadRegister(threadName);
      for (unsigned long i = 0; i < 10000000; i++) {
         getpid();
      }
      EXPECT_EQ(0, mProcStats.GetThreadUserPercent(syscall(SYS_gettid)));
      EXPECT_EQ(0, mProcStats.GetThreadSystemPercent(syscall(SYS_gettid)));
      mProcStats.Update();
      std::map<std::string, pid_t> registeredThreads;
      std::stringstream hiddenName;
      hiddenName << threadName << "_" << 0;
      mProcStats.GetRegisteredThreads(registeredThreads);
      std::map<std::string, pid_t>::iterator myRegistration =
              registeredThreads.find(hiddenName.str());
      ASSERT_NE(registeredThreads.end(), myRegistration);
      EXPECT_EQ(syscall(SYS_gettid), myRegistration->second);
//      for (std::map<std::string, pid_t>::iterator it = registeredThreads.begin();
//              it != registeredThreads.end() ; it++) {
//         std::cout << it->first << " " << mProcStats.GetThreadUserPercent(it->second) << std::endl ;
//      }
      EXPECT_NE(0, mProcStats.GetThreadUserPercent(myRegistration->second));
      //EXPECT_NE(0,mProcStats.GetThreadSystemPercent(myRegistration->second));
   }
}

TEST_F(ProcStatsTest, ThreadReRegistration) {
   std::string threadName("TestThreadTwo");
   mProcStats.ThreadRegister(threadName);
   std::string nextThreadName = "TestThreadThree";
   mProcStats.ThreadRegister(nextThreadName);
   std::map<std::string, pid_t> registeredThreads;
   mProcStats.GetRegisteredThreads(registeredThreads);
   std::stringstream hiddenName;
   hiddenName << threadName << "_" << 0;
   std::map<std::string, pid_t>::iterator myRegistration =
           registeredThreads.find(hiddenName.str());
   ASSERT_EQ(registeredThreads.end(), myRegistration);
   hiddenName.str("");
   hiddenName << nextThreadName << "_" << 0;
   myRegistration =
           registeredThreads.find(hiddenName.str());
   ASSERT_NE(registeredThreads.end(), myRegistration);
}

