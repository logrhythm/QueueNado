#pragma once

#include "CommandProcessor.h"
#include "CommandFactory.h"
class MockCommandProcessor : public CommandProcessor {
public:
   explicit MockCommandProcessor(const Conf& conf) : CommandProcessor(conf) {}
   virtual ~MockCommandProcessor() {}
   void ChangeRegistration(const protoMsg::CommandRequest_CommandType type, CommandFactory::CreationCallback callback) {
      mCommandFactory.UnregisterCommand(type);
      mCommandFactory.RegisterCommand(type,callback);
   }
   CommandFactory::CreationCallback CheckRegistration(const protoMsg::CommandRequest_CommandType type) {
      return mCommandFactory.GetCommandCallback(type);
   }
   void KillCommandsThatWillNeverFinish(unsigned int maxTimeInSeconds,
                                        std::map<std::string, std::tuple<std::weak_ptr<Command>, time_t, pthread_t> >& runningAsyncCommands) {
      CommandProcessor::KillCommandsThatWillNeverFinish(maxTimeInSeconds,runningAsyncCommands);
   }
   
   void SetTimeout(const unsigned int timeout) {
      CommandProcessor::SetTimeout(timeout);
   }

};
