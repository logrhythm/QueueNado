#pragma once

#include "gtest/gtest.h"
#include "MockProcessClientCommand.h"
#include "CommandRequest.pb.h"
#include "MockConf.h"

class PCapFilePathCommandTest : public ::testing::Test {
public:

   PCapFilePathCommandTest() : autoManagedManager(conf){}

protected:
   virtual void SetUp() {
      cmd.set_type(protoMsg::CommandRequest_CommandType_PCAP_FILE_PATH);
      
   }

   virtual void TearDown() {}
   
   protoMsg::CommandRequest cmd;
   MockConf conf;
   MockProcessClientCommand autoManagedManager;
};

 
