#include "ProcStatsTest.h"
#include "MockProcStats.h"
#include "FileIO.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 
#include <sys/syscall.h> 
#include <g2log.hpp>

TEST_F(ProcStatsTest, ValgrindTest) {
   ProcStats & procStats(ProcStats::Instance());
   procStats.ThreadRegister("TEST");
   int iterations=100;
   while (iterations-- > 0) {
            procStats.Update();
   }
}
TEST_F(ProcStatsTest, ConstructSingleton) {
   if (geteuid() == 0) {
      EXPECT_EQ(0, mProcStats.GetUserPercent());
      EXPECT_EQ(0, mProcStats.GetIOWaitPercent());
      EXPECT_EQ(0, mProcStats.GetSystemPercent());
   }
}



TEST_F(ProcStatsTest, VerifyTestResources) {
#ifdef LR_DEBUG
   auto stat_1 = FileIO::ReadAsciiFileContent("resources/stat.1");
   auto stat_2 = FileIO::ReadAsciiFileContent("resources/stat.2");
   auto error = FileIO::ReadAsciiFileContent("resources/not_existing_file.stat.1");

   EXPECT_FALSE(stat_1.HasFailed());
   EXPECT_FALSE(stat_2.HasFailed());
   EXPECT_TRUE(error.HasFailed());
#endif 
}




TEST_F(ProcStatsTest, ReadValidCPUStatFile) {
#ifdef LR_DEBUG
   CpuJiffies jiffies("resources/stat.1");
   EXPECT_EQ(jiffies.numberOfCores, 8);
   EXPECT_TRUE(jiffies.success);
   
   CpuJiffies jiffies_2("resources/stat.2");
   EXPECT_EQ(jiffies_2.numberOfCores, 8);
   EXPECT_EQ(jiffies_2.success, jiffies.success);
#endif    
} 

TEST_F(ProcStatsTest, CpuJiffiesInitialization) {
#ifdef LR_DEBUG
   CpuJiffies jiffies;
   EXPECT_EQ(jiffies.Sum(), 0);
   EXPECT_TRUE(jiffies.success);
#endif    
}

TEST_F(ProcStatsTest, CpuJiffiesDeltaRightAndWrong) {
#ifdef LR_DEBUG
  CpuJiffies older("resources/stat100.1");
  CpuJiffies newer("resources/stat100.2");
  EXPECT_TRUE(CpuJiffies::Delta(newer, older) > 0) << newer.Sum() << " : " << older.Sum();
  EXPECT_EQ(0, CpuJiffies::Delta(older, newer)) << older.Sum() << " : " << newer.Sum();
#endif    
}

TEST_F(ProcStatsTest, CpuJiffiesCopies) {
   CpuJiffies cpuSystem("resources/stat100.1");
   CpuJiffies cpuCopy = cpuSystem;
   CpuJiffies testCopy;
   EXPECT_EQ(testCopy.Sum(), 0);
   testCopy = cpuSystem;
   EXPECT_NE(testCopy.Sum(), 0);
   EXPECT_EQ(testCopy.Sum(), cpuSystem.Sum());

   EXPECT_EQ(cpuCopy.Sum(), cpuSystem.Sum());
   EXPECT_EQ(cpuCopy.numberOfCores, cpuSystem.numberOfCores);
   EXPECT_EQ(cpuCopy.success, cpuSystem.success);   
}


TEST_F(ProcStatsTest, ThreadJiffiesCopies) {   
   ThreadJiffies cpuThread("resources/task.stat100.2");
   ThreadJiffies copy = cpuThread;
   
   EXPECT_EQ(cpuThread.processName, copy.processName);
   EXPECT_EQ(cpuThread.userTime, copy.userTime);
   EXPECT_EQ(cpuThread.systemTime, copy.systemTime); 
   EXPECT_EQ(cpuThread.success, copy.success); 
}


// This was is verifying the old way of calculating thread cpu consumtion
// It is 'somewhat wrong since you do get the the exact precise result
// See also test below: VerifyThreadCpuCalculationsOnSystem
TEST_F(ProcStatsTest, VerifyThreadCpuCalculationsOldStyle) {
#ifdef LR_DEBUG
   ThreadJiffies cpuThread1("resources/task.stat100.1");
   CpuJiffies cpuSystem1("resources/stat100.1");

   ThreadJiffies cpuThread2("resources/task.stat100.2");
   CpuJiffies cpuSystem2("resources/stat100.2");

   float threadDiff = cpuThread2.userTime - cpuThread1.userTime + cpuThread2.systemTime - cpuThread1.systemTime;
   float deltaJiffies = CpuJiffies::Delta(cpuSystem2, cpuSystem1); // i.e. old was ignoring the actual core
   EXPECT_TRUE(deltaJiffies > 0);
   
   // it is not "right" to calculate with 'times number of cores' but it gives
   // a rough estimation which should correspond to the real test below
   // 'VerifyThreadCpuCalulationOnSystem'
   size_t percentUsage = (100*threadDiff)/deltaJiffies;
   size_t maxCoreCpuConsumption = 100/16; // 16-core system
   EXPECT_EQ(percentUsage, maxCoreCpuConsumption); // verified by manually calculating
#endif
}
   
TEST_F(ProcStatsTest, VerifyThreadCpuCalculationsOnSystem) {
#ifdef LR_DEBUG
   ThreadJiffies cpuThread1("resources/task.stat100.1");
   CpuJiffies cpuSystem1("resources/stat100.1");
   
   ThreadJiffies cpuThread2("resources/task.stat100.2");
   CpuJiffies cpuSystem2("resources/stat100.2");
   
   // verify that for the specific test the core is on : 1
   const size_t coreID = 1; 
   
   EXPECT_EQ(cpuThread1.processName, cpuThread2.processName);
   EXPECT_TRUE(cpuThread1.success == cpuThread2.success);
   
   EXPECT_EQ(cpuSystem1.numberOfCores, cpuSystem2.numberOfCores);
   EXPECT_EQ(cpuSystem1.numberOfCores, 16); // 16 cores, ref the files
   
   // Calculate actual CPU usage
   auto numberOfCores = cpuSystem2.numberOfCores;
   auto deltaSystemJiffies = CpuJiffies::Delta(cpuSystem2, cpuSystem1);
   auto deltaThreadJiffies = cpuThread2.userTime-cpuThread1.userTime + cpuThread2.systemTime-  cpuThread1.systemTime;
   auto usageInPercentUnits = numberOfCores * 100* deltaThreadJiffies/deltaSystemJiffies;
   EXPECT_EQ(100, usageInPercentUnits); // ref test above: "VerifyThreadCpuCalculationsOldStyle"
#endif  
}

TEST_F(ProcStatsTest, UpdateCpuStats_FromZero) {
#ifdef LR_DEBUG

   CpuJiffies cpuZero;
   cpuZero.numberOfCores = 1;
   EXPECT_EQ(cpuZero.Sum(), 0);
   EXPECT_TRUE(cpuZero.success);

   MockProcStats usageStats;
   usageStats.mUsePseudoCpuJiffies = true;
   usageStats.mPseudoCpuJiffies = cpuZero;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with zero values 

   // Only zeroes. keeps % as zero
   EXPECT_EQ(0, usageStats.GetUserPercent());
   EXPECT_EQ(0, usageStats.GetIOWaitPercent());
   EXPECT_EQ(0, usageStats.GetSystemPercent());


   CpuJiffies cpuSystem1("resources/stat100.1");
   usageStats.mPseudoCpuJiffies = cpuSystem1;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with real values 

   // from zero to real values .... use zero %
   EXPECT_EQ(0, usageStats.GetUserPercent());
   EXPECT_EQ(0, usageStats.GetIOWaitPercent());
   EXPECT_EQ(0, usageStats.GetSystemPercent());


   CpuJiffies cpuSystem2("resources/stat100.2");
   usageStats.mPseudoCpuJiffies = cpuSystem1;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with real values 

   // from real values to real values .... use above zero %
   EXPECT_EQ(0, usageStats.GetUserPercent());
   EXPECT_EQ(0, usageStats.GetIOWaitPercent());
   EXPECT_EQ(0, usageStats.GetSystemPercent());
   EXPECT_LT(usageStats.GetUserPercent(), 100000);
   EXPECT_LT(usageStats.GetIOWaitPercent(), 100000);
   EXPECT_LT(usageStats.GetSystemPercent(), 100000);

#endif
}

TEST_F(ProcStatsTest, UpdateCpuStats_ToZero) {
#ifdef LR_DEBUG

   MockProcStats usageStats;
   EXPECT_EQ(0, usageStats.GetUserPercent());
   EXPECT_EQ(0, usageStats.GetIOWaitPercent());
   EXPECT_EQ(0, usageStats.GetSystemPercent());


   CpuJiffies cpuSystem1("resources/stat100.1");
   usageStats.mUsePseudoCpuJiffies = true;
   usageStats.mPseudoCpuJiffies = cpuSystem1;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with values 

   // Only zeroes. keeps % as zero
   EXPECT_EQ(0, usageStats.GetUserPercent());
   EXPECT_EQ(0, usageStats.GetIOWaitPercent());
   EXPECT_EQ(0, usageStats.GetSystemPercent());


   CpuJiffies cpuZero;
   cpuZero.numberOfCores = cpuSystem1.numberOfCores;
   usageStats.mPseudoCpuJiffies = cpuZero;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with real values 

   // from real to zero : use zero %
   EXPECT_EQ(0, usageStats.GetUserPercent());
   EXPECT_EQ(0, usageStats.GetIOWaitPercent());
   EXPECT_EQ(0, usageStats.GetSystemPercent());


   // from real to real : get above zero percent
   usageStats.mPseudoCpuJiffies = cpuSystem1;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with real values from 1 
   EXPECT_EQ(true, usageStats.mPseudoCpuJiffies.success); // updating with real values from 1 

   EXPECT_EQ(0, usageStats.GetUserPercent());
   EXPECT_EQ(0, usageStats.GetIOWaitPercent());
   EXPECT_EQ(0, usageStats.GetSystemPercent());


   CpuJiffies cpuSystem2("resources/stat100.2");
   usageStats.mPseudoCpuJiffies = cpuSystem2;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with real values from 2

   // from zero to real values ... use above zero %
   EXPECT_GT(usageStats.GetUserPercent(), 0);
   EXPECT_GT(usageStats.GetIOWaitPercent(), 0);
   EXPECT_GT(usageStats.GetSystemPercent(), 0);
   EXPECT_LT(usageStats.GetUserPercent(), 100000);
   EXPECT_LT(usageStats.GetIOWaitPercent(), 100000);
   EXPECT_LT(usageStats.GetSystemPercent(), 100000);
#endif
}


TEST_F(ProcStatsTest, UpdateCpuStats_CrazyNumbers) {
#ifdef LR_DEBUG

   MockProcStats usageStats(false, false, 0, true);
   CpuJiffies cpuZero;
   cpuZero.numberOfCores = 1;
   EXPECT_TRUE(cpuZero.success);
   usageStats.mUsePseudoCpuJiffies = true;
   usageStats.mPseudoCpuJiffies = cpuZero;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with zero values   
   

   
   CpuJiffies cpuSystem1("resources/stat100.1");
   EXPECT_TRUE(cpuSystem1.success);
   usageStats.mPseudoCpuJiffies = cpuSystem1;
   EXPECT_EQ(true, usageStats.UpdateCpuStats());
   EXPECT_EQ(usageStats.GetUserPercent(), 0);
   EXPECT_EQ(usageStats.GetIOWaitPercent(), 0);
   EXPECT_EQ(usageStats.GetSystemPercent(), 0);
   EXPECT_EQ(cpuSystem1.Sum(), 1504279949);


   CpuJiffies cpuSystem2("resources/stat100.2");
   EXPECT_EQ(cpuSystem2.Sum(), 1504287888);

   EXPECT_TRUE(cpuSystem2.Sum() > cpuSystem1.Sum()) << cpuSystem2.Sum() << ":" << cpuSystem1.Sum();

   usageStats.mPseudoCpuJiffies = cpuSystem2;
   EXPECT_EQ(true, usageStats.UpdateCpuStats()); // updating with real values from 2

   // from zero to real values ... use above zero %
   EXPECT_NE(usageStats.GetUserPercent(), 0);
   EXPECT_NE(usageStats.GetIOWaitPercent(), 0);
   EXPECT_NE(usageStats.GetSystemPercent(), 0);
   EXPECT_LT(usageStats.GetUserPercent(), 100000);
   EXPECT_LT(usageStats.GetIOWaitPercent(), 100000);
   EXPECT_LT(usageStats.GetSystemPercent(), 100000);

   CpuJiffies cpuWeird;
   cpuWeird.numberOfCores =cpuSystem2.numberOfCores;
   const int highValue = -1;
   cpuWeird.user = cpuSystem2.user * 10;
   cpuWeird.nice = cpuSystem2.nice * 10;
   cpuWeird.system = cpuSystem2.system * 10;
   cpuWeird.idle = cpuSystem2.idle * 10;
   cpuWeird.irq = cpuSystem2.irq * 10;
   cpuWeird.softirq = cpuSystem2.irq * 10;
   cpuWeird.iowait = cpuSystem2.iowait * highValue;
   cpuWeird.success = true;

   usageStats.mPseudoCpuJiffies = cpuWeird;
   EXPECT_EQ(false, usageStats.UpdateCpuStats());  // updating with weird values which failed
   
   // Measure crazy ... should ignore it all and reset back to zero
   EXPECT_EQ(usageStats.GetUserPercent(), 0);
   EXPECT_EQ(usageStats.GetIOWaitPercent(), 0);
   EXPECT_EQ(usageStats.GetSystemPercent(), 0);
   
  
   usageStats.mPseudoCpuJiffies = cpuSystem1; // coping new values
   EXPECT_EQ(true, usageStats.UpdateCpuStats());  // updating with real values
   EXPECT_EQ(usageStats.GetUserPercent(), 0);
   EXPECT_EQ(usageStats.GetIOWaitPercent(), 0);
   EXPECT_EQ(usageStats.GetSystemPercent(), 0);


   // Back to normal
   usageStats.mPseudoCpuJiffies = cpuSystem2; // coping new values
   EXPECT_EQ(true, usageStats.UpdateCpuStats());  // updating with ++ values
   EXPECT_GT(usageStats.GetUserPercent(), 0);
   EXPECT_GT(usageStats.GetIOWaitPercent(), 0);
   EXPECT_GT(usageStats.GetSystemPercent(), 0);
   EXPECT_LT(usageStats.GetUserPercent(), 100000);
   EXPECT_LT(usageStats.GetIOWaitPercent(), 100000);
   EXPECT_LT(usageStats.GetSystemPercent(), 100000);
   
#endif  
}

TEST_F(ProcStatsTest, VerifyCompleteThreadCpuCalculations) {
#ifdef LR_DEBUG
    MockProcStats procStats;
    procStats.SetStatFile({"resources/stat100.1"});
    procStats.SetTaskPseudoFile({"resources/task.stat100.1"});
    procStats.ThreadRegister({"looping"}); // ref task.stat100.1 and .2
    procStats.Update();

    procStats.SetStatFile("resources/stat100.2");
    procStats.SetTaskPseudoFile({"resources/task.stat100.2"});
    procStats.Update();

    std::map<std::string, pid_t> registeredThreads;
    procStats.GetRegisteredThreads(registeredThreads);
    for (auto& part : registeredThreads) {
        LOG(DEBUG) << "found: " << part.first << ", pid: " << part.second;
    }
    ASSERT_EQ(registeredThreads.size(), 1);


    auto myRegistration = registeredThreads.find({"looping_0"});
    ASSERT_NE(registeredThreads.end(), myRegistration);

    size_t userPercent = procStats.GetThreadUserPercent(myRegistration->second);
    size_t systemPercent = procStats.GetThreadSystemPercent(myRegistration->second);

    EXPECT_NE(0, userPercent);
    size_t fullUtlization = 1 * 100000; // i.e. 100% modified for www and protobuffer sending
    EXPECT_EQ(fullUtlization, userPercent + systemPercent);
#endif  
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
      std::map<std::string, pid_t> registeredThreads;
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
   std::map<std::string, pid_t> registeredThreads;
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

TEST_F(ProcStatsTest, ProcStatsFromFile) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetStatFile("resources/stat.1");
   ASSERT_TRUE(procStats.UpdateSystemCPU().success);

#endif
}
TEST_F(ProcStatsTest, ProcStatsFromFileNotExist) {
#ifdef LR_DEBUG
   MockProcStats procStats;
   procStats.SetStatFile("resources/FILE_NOT_FOUND");
   ASSERT_FALSE(procStats.UpdateSystemCPU().success);

#endif
}
