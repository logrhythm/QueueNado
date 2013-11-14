#pragma once

#include "gtest/gtest.h"
#include "MockProcessManagerCommand.h"
#include "CommandRequest.pb.h"
#include "MockConf.h"


class DriveInfoCommandTest : public ::testing::Test {
public:

   DriveInfoCommandTest() {
   };

protected:

   virtual void SetUp() {
      autoManagedManager = new MockProcessManagerCommand(conf);
      cmd.set_type(protoMsg::CommandRequest_CommandType_DRIVEINFO);
   }

   virtual void TearDown() {
   }
   MockProcessManagerCommand* autoManagedManager;
   protoMsg::CommandRequest cmd;
   MockConf conf;
};
