

#pragma once

#include "ShutdownCommand.h"
#include "Command.h"
#include "MockProcessManager.h"
#include <g2log.hpp>

class MockShutdownCommand : public ShutdownCommand {
public:
   using ShutdownCommand::DoTheShutdown;
   static bool wasShutdownCalled;
   static bool callRealShutdownCommand;
   static const MockConf mockConf;
   static MockProcessManagerNoInit mockProcessManagerNoInit;

   MockShutdownCommand(const protoMsg::CommandRequest& request, ProcessManager& processManager)
   : ShutdownCommand(request, processManager) {
   }

   virtual ~MockShutdownCommand() {
   }

   // NEVER, use this function unless you are absolutely sure what will happen. It will
   // create the REAL ProcessManager which WILL execute the command if 
   // 'callRealShutdownCommand' is set to true. If it is then your PC will shut off.

   static std::shared_ptr<Command> FatalAndDangerousConstruct(const protoMsg::CommandRequest& request) {
      const MockConf conf;
      std::shared_ptr<Command> command;
      if (callRealShutdownCommand) {
         command.reset(new MockShutdownCommand(request, ProcessManager::Instance()));
      } else {

         command.reset(new MockShutdownCommand(request, mockProcessManagerNoInit));
      }
      return command;
   }

   void DoTheShutdown() LR_OVERRIDE {
      MockShutdownCommand::wasShutdownCalled = true;
      if (callRealShutdownCommand) {

         ShutdownCommand::DoTheShutdown();
      }
      LOG(INFO) << "Pseudo run of 'DoTheShutdown'";
      return;
   }
};

bool MockShutdownCommand::wasShutdownCalled = false;
bool MockShutdownCommand::callRealShutdownCommand = false;
const MockConf MockShutdownCommand::mockConf{};
MockProcessManagerNoInit MockShutdownCommand::mockProcessManagerNoInit{MockShutdownCommand::mockConf};