#pragma once

#include "gtest/gtest.h"
#include "MockProcessManagerCommand.h"
#include "CommandRequest.pb.h"
#include "MockConf.h"


class DriveInfoCommandTest : public ::testing::Test {
public:

   DriveInfoCommandTest() {};

protected:
   virtual void SetUp() {
      autoManagedManager = new MockProcessManagerCommand(conf);
      cmd.set_type(protoMsg::CommandRequest_CommandType_DRIVEINFO);
   }

   virtual void TearDown() {}
   
   bool Compare(const DriveInfo& first, const DriveInfo& second);
   MockProcessManagerCommand* autoManagedManager;
   protoMsg::CommandRequest cmd;
   MockConf conf;
};

  
 bool DriveInfoCommandTest::Compare(const DriveInfo& first, const DriveInfo& second) {
    std::string firstMsg;
    first.SerializeToString(&firstMsg);
    
    std::string secondMsg;
    second.SerializeToString(&secondMsg);
    
    LOG_IF(DEBUG, !(firstMsg == secondMsg)) << "\n: 1 : " << firstMsg << "\n: 2 : " << secondMsg;
    return (firstMsg == secondMsg);
 }

 
//     bool driveCompare = true;
//    driveCompare = driveCompare && (first.has_model() == second.has_model());
//    driveCompare = driveCompare && (first.has_devicel() == second.has_model());
//    driveCompare = driveCompare && (first.has_capacityinmb() == second.has_model());
//    driveCompare = driveCompare && (first.partitions_size() == second.partitions_size());
//    driveCompare = driveCompare && (first.has_table() == second.has_table());
//    
//    if (false == driveCompare) {
//       return driveCompare;
//    }
//    
//    bool partitionCompare = true;
//    const size_t numberOfPartitions = first.partitions_size();
//    for (size_t index = 0; partitionCompare && (index < numberOfPartitions); ++index) {
//       const auto& firstPartition = first.partitions(index);
//       const auto& secondPartition = second.partitions(index);
//       partitionCompare = partitionCompare && Compare(firstPartition, secondPartition);
//    }
//    
//    return partitionCompare;