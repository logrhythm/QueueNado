
#pragma once

#include <gtest/gtest.h>
#include <czmq.h>

#include "MockProcessManagerCommand.h"
#include "ProcessManager.h"
#include "MockConf.h"
#include "CommandRequest.pb.h"

class ConfigRequestCommandTest : public ::testing::Test {
public:
   ConfigRequestCommandTest() {
      zctx_interrupted = false;
   }

protected:
 void SetUp() {
      autoManagedManager = new MockProcessManagerCommand(conf);
      cmd.set_type(protoMsg::CommandRequest_CommandType_CONFIG_REQUEST);
   };

   virtual void TearDown() {}

   const MockConf conf;
   MockProcessManagerCommand* autoManagedManager;
   protoMsg::CommandRequest cmd;
};

