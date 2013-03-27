#include "ProcessManagerTest.h"
#include "MockProcessManager.h"
#include "MockConf.h"
#include <sstream>
#include "g2logworker.hpp"
#include "g2log.hpp"
TEST_F(ProcessManagerTest, RegisterDaemonWithEnv) {
#ifdef LR_DEBUG
   MockConf conf;
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   ProcessManager sendManager(conf);
   EXPECT_TRUE(sendManager.Initialize());
   std::string processName("/bin/sh");
   std::string processArgs;
   std::map<std::string, std::string> env;
   env["a"] = "b";
   processArgs = "resources/sleepIfA.sh";
   EXPECT_TRUE(sendManager.RegisterProcessWithEnvironment(processName, processArgs, env));
   processName = "/bin/ps";
   processArgs = "-ef |grep -v grep | grep \"/bin/sh\" | grep ";
   testQueue.str("");
   testQueue << getpid();
   processArgs += testQueue.str();
   std::string result = (sendManager.RunProcess(processName, processArgs));   
   LOG(DEBUG) << result;
   EXPECT_NE(std::string::npos, result.find(testQueue.str()));
   EXPECT_NE(std::string::npos, result.find("/bin/sh"));
   raise(SIGTERM);
   testManager.DeInit();
#endif
}
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
   ProcessManager* testManagerPoint = new ProcessManager(conf);
   delete testManagerPoint;
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
TEST_F(ProcessManagerTest, FailInitializationFromAnotherObject) {
#ifdef LR_DEBUG
   MockConf conf;
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   conf.mProcessManagmentQueue = "invalid";
   ProcessManager sendManager(conf);
   EXPECT_FALSE(sendManager.Initialize());
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
   processArgs = "2";
   EXPECT_TRUE(sendManager.RegisterProcess(processName, processArgs));
   processName = "/bin/ps";
   processArgs = "-ef | grep \"/bin/sleep\" | grep -v grep | grep ";
   testQueue.str("");
   testQueue << getpid();
   processArgs += testQueue.str();
   std::string result = (sendManager.RunProcess(processName, processArgs));
   LOG(DEBUG) << result;
   EXPECT_NE(std::string::npos, result.find(testQueue.str()));
   EXPECT_NE(std::string::npos, result.find("/bin/sleep"));
   std::this_thread::sleep_for(std::chrono::seconds(3));
   std::string result2 = (sendManager.RunProcess(processName, processArgs));
   LOG(DEBUG) << result2;
   EXPECT_NE(std::string::npos, result2.find(testQueue.str()));
   EXPECT_NE(std::string::npos, result2.find("/bin/sleep"));  
   EXPECT_NE(result,result2);
   EXPECT_TRUE(sendManager.UnRegisterProcess("/bin/sleep"));
   result2 = (sendManager.RunProcess(processName, processArgs));
   LOG(DEBUG) << result2;
   EXPECT_EQ(std::string::npos, result2.find(testQueue.str()));
   EXPECT_EQ(std::string::npos, result2.find("/bin/sleep"));  
   raise(SIGTERM);
   testManager.DeInit();
#endif
}

TEST_F(ProcessManagerTest, RegisterDaemonCleanup) {
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
   processArgs = "2";
   EXPECT_TRUE(sendManager.RegisterProcess(processName, processArgs));
   processName = "/bin/ps";
   processArgs = "-ef | grep \"/bin/sleep\" | grep -v grep | grep ";
   testQueue.str("");
   testQueue << getpid();
   processArgs += testQueue.str();
   std::string result = (sendManager.RunProcess(processName, processArgs));
   LOG(DEBUG) << result;
   EXPECT_NE(std::string::npos, result.find(testQueue.str()));
   EXPECT_NE(std::string::npos, result.find("/bin/sleep"));
   raise(SIGTERM);
   testManager.DeInit();
   LOG(DEBUG) << "Trying to re-initialize";
   zctx_interrupted = false;
   EXPECT_TRUE(testManager.Initialize());
   result = (sendManager.RunProcess(processName, processArgs));
   LOG(DEBUG) << result;
   EXPECT_EQ(std::string::npos, result.find(testQueue.str()));
   EXPECT_EQ(std::string::npos, result.find("/bin/sleep"));  
   raise(SIGTERM);
   testManager.DeInit();
#endif
}
TEST_F(ProcessManagerTest, RegisterDaemonFails) {
#ifdef LR_DEBUG
   MockConf conf;
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
   MockProcessManager testManager(conf);
   testManager.mExecFails = true;
   EXPECT_TRUE(testManager.Initialize());
   ProcessManager sendManager(conf);
   EXPECT_TRUE(sendManager.Initialize());
   std::string processName("/bin/sleep");
   std::string processArgs;
   processArgs = "2";
   EXPECT_FALSE(sendManager.RegisterProcess(processName, processArgs));
   processName = "/bin/ps";
   processArgs = "-ef | grep \"/bin/sleep\" | grep -v grep | grep ";
   testQueue.str("");
   testQueue << getpid();
   processArgs += testQueue.str();
   std::string result = (sendManager.RunProcess(processName, processArgs));
   LOG(DEBUG) << result;
   EXPECT_EQ(std::string::npos, result.find(testQueue.str()));
   EXPECT_EQ(std::string::npos, result.find("/bin/sleep"));
   raise(SIGTERM);
   testManager.DeInit();
#endif
}
TEST_F(ProcessManagerTest, RegisterDaemonKillFails) {
#ifdef LR_DEBUG
   MockConf conf;
   std::stringstream testQueue;
   testQueue << "ipc:///tmp/ProcessManagerTest." << getpid();
   conf.mProcessManagmentQueue = testQueue.str();
   MockProcessManager testManager(conf);
   testManager.mKillFails = true;
   EXPECT_TRUE(testManager.Initialize());
   ProcessManager sendManager(conf);
   EXPECT_TRUE(sendManager.Initialize());
   std::string processName("/bin/sleep");
   std::string processArgs;
   processArgs = "1";
   EXPECT_TRUE(sendManager.RegisterProcess(processName, processArgs));
   std::this_thread::sleep_for(std::chrono::seconds(1));  
   EXPECT_FALSE(sendManager.UnRegisterProcess("/bin/sleep"));
   raise(SIGTERM);
   testManager.DeInit();
#endif
}