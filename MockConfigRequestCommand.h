
#pragma once
#include "ConfigRequestCommand.h"
#include "include/global.h"
#include "gmock/gmock.h"


struct MockConfigRequestCommand : public ConfigRequestCommand {
   MockConfigRequestCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager)
   : ConfigRequestCommand(request, processManager) { }

   
   virtual ~MockConfigRequestCommand(){}
};

class GMockConfigRequestCommand : public MockConfigRequestCommand {
   public:
      GMockConfigRequestCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager) :
       MockConfigRequestCommand(request, processManager){}
       
      virtual ~GMockConfigRequestCommand() = default;
};
