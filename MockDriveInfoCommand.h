#pragma once

#include "DriveInfoCommand.h"
#include "include/global.h"
#include "DriveInfo.h"
#include <vector>
#include <string>
class MockDriveInfoCommand : public DriveInfoCommand {
public:
   explicit MockDriveInfoCommand(const protoMsg::CommandRequest& request, const std::string& programName) : DriveInfoCommand(request,programName) {
      
   }
   MockDriveInfoCommand(const protoMsg::CommandRequest& request,
                        ProcessManager* processManager, const std::string& programName) : DriveInfoCommand(request,processManager,programName) {
      
   }
   ~MockDriveInfoCommand() {}
   
   void GetDiskInfo(protoMsg::CommandReply& reply) {
     DriveInfoCommand::GetDiskInfo(reply); 
   }
   
   
   std::vector<DriveInfo> ExtractPartedToDrives(const std::string& parted) const LR_OVERRIDE {
      return DriveInfoCommand::ExtractPartedToDrives(parted);
   }
};
