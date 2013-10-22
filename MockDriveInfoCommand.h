#pragma once

#include "DriveInfoCommand.h"

class MockDriveInfoCommand : public DriveInfoCommand {
public:
   explicit MockDriveInfoCommand(const protoMsg::CommandRequest& request) : DriveInfoCommand(request) {
      
   }
   MockDriveInfoCommand(const protoMsg::CommandRequest& request,
                        ProcessManager* processManager) : DriveInfoCommand(request,processManager) {
      
   }
   ~MockDriveInfoCommand() {}
   
   std::string GetSingleDiskPartedArgs(const std::string& diskId) {
      return DriveInfoCommand::GetSingleDiskPartedArgs(diskId);
   }
};
