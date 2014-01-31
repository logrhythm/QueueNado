#pragma once

#include "gtest/gtest.h"
#include "CommandProcessor.h"
#include "MockConf.h"
#include "MockCommandProcessor.h"

#include <boost/thread.hpp>
#include <czmq.h>
#include <thread>
#include <chrono>
#include <boost/lexical_cast.hpp>

class CommandProcessorTests : public ::testing::Test {
public:

   CommandProcessorTests() {
      zctx_interrupted = false;
      srand(std::time(NULL));
   }
protected:

   virtual void SetUp() {
      conf.mCommandQueue = "tcp://127.0.0.1:";
      conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   };

   virtual void TearDown() {  }

   
   
   // The "KillCommandsThatWillNeverFinish" will be called
   // periodically in the CommandProcessor. This function
   // Ensures that has run at least three times before
   // exiting (or maximum wait of 10s)
   // 
   // It is by design that it checks the "KillCommands"... function
   // and not the CommadnProcessor::CommandReader (thread loop)
   // due to branching logic
   bool WaitForKillCommandsThatWillNeverFinish(MockCommandProcessor& testProcessor) {
      using namespace std::chrono;
      auto start = high_resolution_clock::now();
      auto getElapsed = [&] {
         return duration_cast<seconds>(high_resolution_clock::now() - start).count();
      };
      // Make sure that in CommandProcessor, the KillCommandsThatWillNeverFinish and 
      // ClearAsyncCommands has both run after the SetTimeout
      // ,. i.e after the third call to KillCommandsThatWillNeverFinish  
      //    we are good to go
      // Worst case scenario
      // --------------------
      // SetTimeout(0)     : CommandProcessor --> Already in KillCommands (count 1)
      //                   : CommandProcessor::KillCommands(count 2)
      // Read counter: 2   :
      //                   : CommandProcessor::CleanAsync
      //   ....
      //                   : CommandProcessor::KillCommands(count 3) --> We are safe
      testProcessor.ResetKillCounter();

      // 2: to avoid race errors with ResetKillCounter
      const size_t enoughKillsToAvoidRace = 3;
      const size_t stopTimeInSeconds{10};

      while (enoughKillsToAvoidRace > testProcessor.GetKillCounter() &&
              getElapsed() < stopTimeInSeconds) {
         std::this_thread::yield();
      }
      return testProcessor.GetKillCounter() >= enoughKillsToAvoidRace;
   }


   MockConf conf;
private:




};

