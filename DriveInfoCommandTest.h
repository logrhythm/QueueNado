#pragma once

#include "gtest/gtest.h"
#include "MockProcessClientCommand.h"
#include "CommandRequest.pb.h"
#include "MockConf.h"

class DriveInfoCommandTest : public ::testing::Test {
public:

   DriveInfoCommandTest() : autoManagedManager(conf){}

protected:
   virtual void SetUp() {
      cmd.set_type(protoMsg::CommandRequest_CommandType_DRIVEINFO);
   }

   virtual void TearDown() {}
   
   bool Compare(const DriveInfo& first, const DriveInfo& second);
   protoMsg::CommandRequest cmd;
   MockConf conf;
   MockProcessClientCommand autoManagedManager;
};

  
 bool DriveInfoCommandTest::Compare(const DriveInfo& first, const DriveInfo& second) {
    std::string firstMsg;
    first.SerializeToString(&firstMsg);
    
    std::string secondMsg;
    second.SerializeToString(&secondMsg);
    
    LOG_IF(DEBUG, !(firstMsg == secondMsg)) << "\n: 1 : " << firstMsg << "\n: 2 : " << secondMsg;
    return (firstMsg == secondMsg);
 }

 
