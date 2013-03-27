#include "ProcessManagerTest.h"
#include "MockConf.h"
#include <sstream>

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
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();

   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   raise(SIGTERM);
   testManager.DeInit();
#endif
}

TEST_F(ProcessManagerTest, RunProcess) {
#ifdef LR_DEBUG
   MockConf conf;
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
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
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
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
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
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
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
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

TEST_F(ProcessManagerTest, RegisterDaemon) {
#ifdef LR_DEBUG
   MockConf conf;
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   ProcessManager sendManager(conf);
   EXPECT_TRUE(sendManager.Initialize());
   std::string processName("/bin/sleep");
   std::string processArgs;
   processArgs = "10";
   EXPECT_TRUE(sendManager.RegisterProcess(processName, processArgs));
   processName = "/bin/ps";
   processArgs = "-ef | grep \"/bin/sleep\" | grep -v grep | grep ";
   testQueue.str("");
   testQueue << getpid();
   processArgs += testQueue.str();
   std::string result = (sendManager.RunProcess(processName, processArgs));
   std::cout << result << std::endl;
   EXPECT_NE(std::string::npos, result.find(testQueue.str()));
   EXPECT_NE(std::string::npos, result.find("/bin/sleep"));
   std::this_thread::sleep_for(std::chrono::seconds(30));
   std::string result2 = (sendManager.RunProcess(processName, processArgs));
   std::cout << result2 << std::endl;
   EXPECT_NE(std::string::npos, result2.find(testQueue.str()));
   EXPECT_NE(std::string::npos, result2.find("/bin/sleep"));  
   EXPECT_NE(result,result2);
   raise(SIGTERM);
   testManager.DeInit();
#endif
}
