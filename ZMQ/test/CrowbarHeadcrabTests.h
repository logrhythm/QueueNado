/* 
 * Author: Robert Weber
 *
 * Created on November 14, 2012, 3:31 PM
 */
#pragma once

#include "gtest/gtest.h"
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include "Headcrab.h"
#include "Crowbar.h"
#include "ZeroMQTests.h"
#include <czmq.h>

class CrowbarHeadcrabTests : public /*::testing::Test*/ ZeroMQTests {
public:

   CrowbarHeadcrabTests() {
   }

   virtual ~CrowbarHeadcrabTests() {
   }

   void Sender(std::string& baseData,int numberOfHits,std::string& binding);
protected:

   virtual void SetUp() {
      std::stringstream makeATarget;
      makeATarget << "ipc:///tmp/ipc.test" << boost::this_thread::get_id();
      mTarget = makeATarget.str();
      srandom((unsigned) time(NULL));
      zctx_interrupted = false;
   }

   virtual void TearDown() {
      raise(SIGTERM);
   }
   std::string mTarget;
};