#include "ProcStatsTest.h"
#include "MockProcStats.h"
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
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetStatFile("resources/stat.1");
   procStats.Update();
   procStats.SetStatFile("resources/stat.2");
   procStats.Update();
   EXPECT_NE(0, procStats.GetUserPercent());
   EXPECT_EQ(11319, procStats.GetUserPercent()); // 11.319% i.e.    0.113199 * 100.000

   EXPECT_NE(0, procStats.GetSystemPercent());
   EXPECT_EQ(900, procStats.GetSystemPercent()); // 0.9% i.e. 0.009004 * 100.000
   
#endif
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
      std::unordered_map<std::string, pid_t> registeredThreads;
      std::stringstream hiddenName;
      hiddenName << threadName << "_" << 0;
      mProcStats.GetRegisteredThreads(registeredThreads);
      auto myRegistration = registeredThreads.find(hiddenName.str());
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
   std::unordered_map<std::string, pid_t> registeredThreads;
   mProcStats.GetRegisteredThreads(registeredThreads);
   std::stringstream hiddenName;
   hiddenName << threadName << "_" << 0;
   auto myRegistration = registeredThreads.find(hiddenName.str());
   ASSERT_EQ(registeredThreads.end(), myRegistration);
   hiddenName.str("");
   hiddenName << nextThreadName << "_" << 0;
   myRegistration = registeredThreads.find(hiddenName.str());
   ASSERT_NE(registeredThreads.end(), myRegistration);
}

TEST_F(ProcStatsTest, MemStatsFromFile) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/meminfo.1");
   ASSERT_TRUE(procStats.UpdateMemStats());

   EXPECT_EQ(10856,procStats.GetFreeMem());
   EXPECT_EQ(15948,procStats.GetTotalMem());
   EXPECT_EQ(4932,procStats.GetUsedMem());
   EXPECT_EQ(160,procStats.GetReservedMem());
   EXPECT_EQ(8039,procStats.GetTotalSwap());
   EXPECT_EQ(7254,procStats.GetFreeSwap());
   EXPECT_EQ(785,procStats.GetUsedSwap());
#endif
}
TEST_F(ProcStatsTest, MemStatsFromFileNotExist) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetMemFile("resources/FILE_NOT_FOUND");
   ASSERT_FALSE(procStats.UpdateMemStats());

#endif
}
TEST_F(ProcStatsTest, ProcStatsFromFile) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetStatFile("resources/stat.1");
   ASSERT_TRUE(procStats.UpdateSystemCPU());

#endif
}
TEST_F(ProcStatsTest, ProcStatsFromFileNotExist) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetStatFile("resources/FILE_NOT_FOUND");
   ASSERT_FALSE(procStats.UpdateSystemCPU());

#endif
}