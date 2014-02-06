#pragma once

#include <gtest/gtest.h>
#include <czmq.h>

#include "MockProcessClientCommand.h"
#include "ProcessClient.h"
#include "MockConf.h"
#include "CommandRequest.pb.h"

class NtpConfigCommandTest : public ::testing::Test {
public:

   NtpConfigCommandTest() : autoManagedManager(conf){
      zctx_interrupted = false;
   }

protected:

   virtual void SetUp() {
      cmd.set_type(protoMsg::CommandRequest_CommandType_NTP_CONFIG);
   };

   virtual void TearDown() {
   }

   const MockConf conf;
   MockProcessClientCommand autoManagedManager;
   protoMsg::CommandRequest cmd;

};

