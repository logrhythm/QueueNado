#pragma once

#include "gtest/gtest.h"
#include "BoomStick.h"
#include <pthread.h>
#include <czmq.h>

class BoomStickTest : public ::testing::Test
{
public:
    BoomStickTest(){
       std::stringstream sS;
       
       sS << "ipc:///tmp/boomsticktest" << pthread_self();
       mAddress = sS.str();
    };

protected:
	virtual void SetUp() {}
	virtual void TearDown() {
      zctx_interrupted = false;
   }
 
   std::string mAddress;
};

