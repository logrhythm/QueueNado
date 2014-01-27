/* 
 * File:   StatsAccumulatorTest.h
 * Author: John Gress
 *
 * Created on December 12, 2012
 */

#pragma once

#include "gtest/gtest.h"
#include "SendStats.h"
#include "Vampire.h"
#include "MockConfMaster.h"
#include "MockConf.h"
#include <string>

namespace networkMonitor {
class MockConfMaster : public ConfMaster {
public:
   MockConfMaster() {
   }
};
}

class StatsAccumulatorTest : public ::testing::Test {
public:

   StatsAccumulatorTest() : mStatSender(0), mAccumulatedStatsRcvQ(0) {
   };

   ~StatsAccumulatorTest() {
   }

   void SetupSendAndRcvQueues( std::string configFilename );
   void TeardownSendAndRcvQueues();
   void SyncWithStatsAccumulator( unsigned int statsIntervalSeconds,
                                  time_t& syncTime );
   void SendStatsToAccumulator( std::string sendStatKey, uint64_t sendStatValue,
                                int numberToSend );
   void VerifyAccumulatedStats( std::string sentStatKey, long sentStatValue,
      int numberSent, time_t expectedTime, unsigned int statsIntervalSeconds );
   void VerifyNoStatsInInterval( unsigned int statsIntervalSeconds );

protected:

   virtual void SetUp() {
   };

   virtual void TearDown() {
   };

   unsigned int mStatsIntervalSeconds;
   MockConfMaster mConfMaster;
   MockConf mConf;

private:
   std::string mStatsAccumulatorQueueName;
   std::string mSendStatsQueueName;
   Vampire* mAccumulatedStatsRcvQ;
   SendStats* mStatSender;

};
