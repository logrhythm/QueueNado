#include "CommandProcessorTests.h"
#include "Conf.h"
#include "MockConf.h"
#include "Headcrab.h"
#include "Crowbar.h"
#include "libconf/Conf.h"
#include "boost/lexical_cast.hpp"


TEST_F(CommandProcessorTests, ConstructAndInitializeFail) {
   MockConf conf;
   conf.mCommandQueue = "invalid";
   CommandProcessor testProcessor(conf);
   EXPECT_FALSE(testProcessor.Initialize());
}
TEST_F(CommandProcessorTests, ConstructAndInitialize) {
   MockConf conf;
   conf.mCommandQueue = "tcp://127.0.0.1:";
   conf.mCommandQueue += boost::lexical_cast<std::string>(rand()%1000 + 20000);
   CommandProcessor testProcessor(conf);
   EXPECT_TRUE(testProcessor.Initialize());
   raise(SIGTERM);
}