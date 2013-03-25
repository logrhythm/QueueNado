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
