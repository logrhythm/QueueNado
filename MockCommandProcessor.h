#pragma once

#include "CommandProcessor.h"
#include "CommandFactory.h"
#include <mutex>


class MockCommandProcessor : public CommandProcessor {
public:
   explicit MockCommandProcessor(const Conf& conf) 
   : CommandProcessor(conf)
   , mKillCounter(0) {}
   
   
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
      
      
      { // explicitly its own scope. increment the kill counter
         std::lock_guard<std::mutex> lock(mMutex);
         mKillCounter++;
      }
   }
   
   
   void SetTimeout(const unsigned int timeout) LR_OVERRIDE {
      CommandProcessor::SetTimeout(timeout);
   }
      
   
   // Test only function
   size_t GetKillCounter() {
      std::lock_guard<std::mutex> lock(mMutex);
      return mKillCounter;
   }

   // Test only function
   void ResetKillCounter() {
      std::lock_guard<std::mutex> lock(mMutex);
      mKillCounter=0;
   }
   
   bool HasWorkerThreadStarted() {
      return CommandProcessor::HasWorkerThreadStarted();
   }
   
   private:
      size_t mKillCounter;
      std::mutex mMutex;
};
