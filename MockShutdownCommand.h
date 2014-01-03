

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
   static std::unique_ptr<ProcessManager> mProcessManager;

   MockShutdownCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager,
           const std::string& programName)
   : ShutdownCommand(request, processManager, programName) {
   }

   virtual ~MockShutdownCommand() {
   }

   // NEVER, use this function unless you are absolutely sure what will happen. It will
   // create the REAL ProcessManager which WILL execute the command if 
   // 'callRealShutdownCommand' is set to true. If it is then your PC will shut off.

   static std::shared_ptr<Command> FatalAndDangerousConstruct(const protoMsg::CommandRequest& request,
           const std::string& programName) {
      const MockConf conf;

      if (mProcessManager.get() == nullptr) {
         if (callRealShutdownCommand) {
            mProcessManager.reset(new ProcessManager(conf, programName));
         } else {
            mProcessManager.reset(new MockProcessManagerNoInit(conf, programName));
         }
      }
      std::shared_ptr<Command> command(new MockShutdownCommand(request, mProcessManager.get(), programName));
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
std::unique_ptr<ProcessManager> MockShutdownCommand::mProcessManager(nullptr);