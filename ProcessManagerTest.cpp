#include "ProcessManagerTest.h"
#include "MockConf.h"

TEST_F(ProcessManagerTest, ConstructAndInitializeFail) {
   MockConf conf;
   conf.mProcessManagmentQueue = "invalid";
   ProcessManager testManager(conf);
   EXPECT_FALSE(testManager.Initialize());
}
TEST_F(ProcessManagerTest, ConstructAndInitialize) {
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   raise(SIGTERM);
}
TEST_F(ProcessManagerTest, RunProcess) {
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   std::string processName("/bin/ls");
   std::string processArgs;
   processArgs = "-l /bin/ls";
   std::string result = (testManager.RunProcess(processName,processArgs));
   EXPECT_NE(std::string::npos,result.find("-rwxr-xr-x. 1 root root"));
   EXPECT_NE(std::string::npos,result.find("/bin/ls"));
   raise(SIGTERM);
}
TEST_F(ProcessManagerTest, RunNonExistantProcess) {
   MockConf conf;
   ProcessManager testManager(conf);
   EXPECT_TRUE(testManager.Initialize());
   std::string processName("/bin/lsssss");
   std::string processArgs;
   processArgs = "-l /bin/ls";
   std::string result = (testManager.RunProcess(processName,processArgs));
   EXPECT_TRUE(result.empty());
   raise(SIGTERM);
}