#pragma once

#include "CommandProcessor.h"

class MockCommandProcessor : public CommandProcessor {
public:

   explicit MockCommandProcessor(const Conf& conf) : CommandProcessor(conf), timeout(3600) {
   }

   virtual ~MockCommandProcessor() {
   }

   void ChangeRegistration(const protoMsg::CommandRequest_CommandType type, CommandFactory::CreationCallback callback) {
      mCommandFactory.UnregisterCommand(type);
      mCommandFactory.RegisterCommand(type, callback);
   }

   CommandFactory::CreationCallback CheckRegistration(const protoMsg::CommandRequest_CommandType type) {
      return mCommandFactory.GetCommandCallback(type);
   }
   
   void KillCommandsThatWillNeverFinish(unsigned int maxTimeInSeconds) {
      CommandProcessor::KillCommandsThatWillNeverFinish(timeout);
   }
   
   unsigned int timeout;
};