#pragma once

#include "gtest/gtest.h"
#include "LuaFunctions.h"
#include "MockLuaFunctions.h"
#include "ConfMaster.h"
#include "ConfSlave.h"

class LuaFunctionsTest : public ::testing::Test {
public:

   LuaFunctionsTest() : conf(networkMonitor::ConfSlave::Instance()), masterConf(networkMonitor::ConfMaster::Instance()){

      masterConf.SetPath("resources/test.yaml");
      masterConf.Start();

      conf.Start();
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   };

protected:

   virtual void SetUp() {
   }

   virtual void TearDown() {
   }
   networkMonitor::ConfMaster& masterConf;
   networkMonitor::ConfSlave& conf;
};

