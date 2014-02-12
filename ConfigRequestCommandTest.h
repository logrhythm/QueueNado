
#pragma once

#include <gtest/gtest.h>
#include <czmq.h>

#include "MockProcessClientCommand.h"
#include "ProcessClient.h"
#include "MockConf.h"
#include "CommandRequest.pb.h"

class ConfigRequestCommandTest : public ::testing::Test {
public:
   ConfigRequestCommandTest() : autoManagedManager(mockConf){
      zctx_interrupted = false;
   }

protected:
 void SetUp() {
      cmd.set_type(protoMsg::CommandRequest_CommandType_CONFIG_REQUEST);
      mockConf = mConfMaster.GetConf()
   };

   virtual void TearDown() {}
   MockConfMaster mConfMaster;
   MockConf mockConf;
   MockProcessClientCommand autoManagedManager;
   protoMsg::CommandRequest cmd;
};

