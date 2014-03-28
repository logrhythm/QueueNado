#pragma once

#include "PCapFilePathCommand.h"
#include "include/global.h"
#include <vector>
#include <string>
class MockPCapFilePathCommand : public PCapFilePathCommand {
public:

   MockPCapFilePathCommand(const protoMsg::CommandRequest& request, ProcessClient& processManager)
   : PCapFilePathCommand(request,processManager) {
      
   }
   ~MockPCapFilePathCommand() {}
   
};
