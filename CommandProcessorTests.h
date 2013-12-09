#pragma once

#include "gtest/gtest.h"
#include "CommandProcessor.h"
#include <czmq.h>
#include <boost/thread.hpp>
#include "MockConf.h"
#include <boost/lexical_cast.hpp>

class CommandProcessorTests : public ::testing::Test {
public:

   CommandProcessorTests() {
      zctx_interrupted = false;
      srand(std::time(NULL));
   };
protected:

   virtual void SetUp() {

      conf.mCommandQueue = "tcp://127.0.0.1:";
      conf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   };

   virtual void TearDown() {

   };
   MockConf conf;
private:

};

