#include "ProcessManagerTest.h"
#include "MockConf.h"

TEST_F(ProcessManagerTest, ConstructAndInitializeFail) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mProcessManagmentQueue = "invalid";
   ProcessManager testManager(conf);
   EXPECT_FALSE(testManager.Initialize());
   testManager.DeInit();
#endif
}

TEST_F(ProcessManagerTest, ConstructAndInitialize) {
#ifdef LR_DEBUG
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   raise(SIGTERM);
   testManager.DeInit();
#endif
}

TEST_F(ProcessManagerTest, RunProcess) {
#ifdef LR_DEBUG
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   std::string processName("/bin/ls");
   std::string processArgs;
   processArgs = "-l /bin/ls";
   std::string result = (testManager.RunProcess(processName, processArgs));
   EXPECT_NE(std::string::npos, result.find("-rwxr-xr-x. 1 root root"));
   EXPECT_NE(std::string::npos, result.find("/bin/ls"));
   raise(SIGTERM);
   testManager.DeInit();
#endif
}

TEST_F(ProcessManagerTest, RunNonExistantProcess) {
#ifdef LR_DEBUG
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   std::string processName("/bin/lsssss");
   std::string processArgs;
   processArgs = "-l /bin/ls";
   std::string result = (testManager.RunProcess(processName, processArgs));
   EXPECT_TRUE(result.empty());
   raise(SIGTERM);
   testManager.DeInit();
#endif
}

TEST_F(ProcessManagerTest, RunProcessFromAnotherObject) {
#ifdef LR_DEBUG
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   ProcessManager sendManager(conf);
   EXPECT_TRUE(sendManager.Initialize());
   std::string processName("/bin/ls");
   std::string processArgs;
   processArgs = "-l /bin/ls";
   std::string result = (sendManager.RunProcess(processName, processArgs));
   EXPECT_NE(std::string::npos, result.find("-rwxr-xr-x. 1 root root"));
   EXPECT_NE(std::string::npos, result.find("/bin/ls"));
   raise(SIGTERM);
   testManager.DeInit();
#endif
}

TEST_F(ProcessManagerTest, RunNonExistantProcessFromAnotherObject) {
#ifdef LR_DEBUG
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   ProcessManager sendManager(conf);
   EXPECT_TRUE(sendManager.Initialize());
   std::string processName("/bin/lsssss");
   std::string processArgs;
   processArgs = "-l /bin/ls";
   std::string result = (sendManager.RunProcess(processName, processArgs));
   EXPECT_TRUE(result.empty());
   raise(SIGTERM);
   testManager.DeInit();
#endif
}