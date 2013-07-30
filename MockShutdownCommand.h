

#pragma once

#include "ShutdownCommand.h"

class MockShutdownCommand : public ShutdownCommand {
public:
   using ShutdownCommand::DoTheShutdown;

   MockShutdownCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager) : ShutdownCommand(request, processManager) {
   }

   virtual ~MockShutdownCommand() {
   }


};