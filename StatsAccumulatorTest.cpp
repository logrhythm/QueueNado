/* * File:   StatsAccumulatorTest.cpp * Author: John Gress
 * 
 * Created on December 12, 2012
 */

#include <string>
#include <csignal>
#include "boost/lexical_cast.hpp"
#include "StatsAccumulatorTest.h"
#include "StatsAccumulator.h"
#include "StatsMsg.pb.h"

void StatsAccumulatorTest::SetupSendAndRcvQueues(std::string configFilename) {
   mConfMaster.SetPath(configFilename);
   mConf = mConfMaster.GetConf();
   mStatsAccumulatorQueueName.clear();
   mStatsAccumulatorQueueName = mConf.GetStatsAccumulatorQueue();
   mSendStatsQueueName.clear();
   mSendStatsQueueName = mConf.GetSendStatsQueue();
   mStatsIntervalSeconds = mConf.GetStatsIntervalSeconds();

   // Expect accumulated stats on the SendStatsQueue
   mAccumulatedStatsRcvQ = new Vampire(mSendStatsQueueName);
   mAccumulatedStatsRcvQ->SetOwnSocket(true);
   mAccumulatedStatsRcvQ->PrepareToBeShot();

   // The Stat Sender sends stats to the accumulator
   mStatSender = new SendStats();
   mStatSender->Initialize(mStatsAccumulatorQueueName);
}

void StatsAccumulatorTest::TeardownSendAndRcvQueues() {
   delete mAccumulatedStatsRcvQ;
   mAccumulatedStatsRcvQ = 0;
   delete mStatSender;
   mStatSender = 0;
}

void StatsAccumulatorTest::SendStatsToAccumulator(std::string sendStatKey,
        uint64_t sendStatValue, int numberToSend) {

   for (int statCnt = 0; statCnt < numberToSend; statCnt++) {
      mStatSender->SendStat(sendStatKey, sendStatValue, true);
   }

}

/**
 * Return as soon at the Stats Accumulator sends an accumulated packet.
 *
 * If the Stats Accumulator thread starts near the end of an interval
 * (intervals are in whole seconds), then the first set of sends to the Stats
 * Accumulator could be divided across two intervals, and the accumulated
 * results would come in two messages.
 *
 * @statsIntervalSeconds - the time interval being used by the Stats
 *                         Accumulator.
 * @syncTime - returns the time in seconds at which the sync occurred.
 */
void StatsAccumulatorTest::SyncWithStatsAccumulator(
        unsigned int statsIntervalSeconds, time_t& syncTime) {

   int shotWaitTimeMs = (statsIntervalSeconds * 1000) + 100;

   std::string sendStatKey("SyncStat");

   // Send one stat
   SendStatsToAccumulator(sendStatKey, 1, 1);

   std::string rcvMsg;
   int count = 0;
   while (!mAccumulatedStatsRcvQ->GetShot(rcvMsg, shotWaitTimeMs)) {
      if (count++ > 10) {
         return;
      }
   }
   do {
      syncTime = std::time(NULL);

      protoMsg::Stats statsMsg;
      statsMsg.ParseFromString(rcvMsg);

      std::string rcvStatKey(statsMsg.key());
      if (rcvStatKey == sendStatKey) {
         long rcvStatValue = statsMsg.longvalue();
         EXPECT_EQ(rcvStatValue, 1);
         return;
      }
   } while (mAccumulatedStatsRcvQ->GetShot(rcvMsg, shotWaitTimeMs));
   EXPECT_TRUE(false);

}

void StatsAccumulatorTest::VerifyAccumulatedStats(std::string sentStatKey,
        long sentStatValue, int numberSent, time_t expectedTime,
        unsigned int statsIntervalSeconds) {

   int shotWaitTimeMs = (statsIntervalSeconds * 1000) + 100;
   long expectedStatValue = sentStatValue * numberSent;

   std::string rcvMsg;
   while (mAccumulatedStatsRcvQ->GetShot(rcvMsg, shotWaitTimeMs)) {
      protoMsg::Stats statsMsg;
      statsMsg.ParseFromString(rcvMsg);

      std::string rcvStatKey(statsMsg.key());
      if (rcvStatKey == sentStatKey) {
         long rcvStatValue = statsMsg.longvalue();
         EXPECT_EQ(rcvStatValue, expectedStatValue);

         uint64_t rcvTimeValue = statsMsg.time();
         EXPECT_TRUE(expectedTime - 1 <= rcvTimeValue && expectedTime + 1 >= rcvTimeValue);
         return;
      }
   }
   EXPECT_TRUE(false);

}

void StatsAccumulatorTest::VerifyNoStatsInInterval(unsigned int statsIntervalSeconds) {
   int shotWaitTimeMs = (statsIntervalSeconds * 1000) + 100;
   std::string rcvMsg;
   EXPECT_FALSE(mAccumulatedStatsRcvQ->GetShot(rcvMsg, shotWaitTimeMs));
}

TEST_F(StatsAccumulatorTest, testConstructor) {
   StatsAccumulator statsAcc(mConf);
}

TEST_F(StatsAccumulatorTest, testNewDelete) {
   StatsAccumulator * pStatsAccumulator = new StatsAccumulator(mConf);
   delete pStatsAccumulator;
}

TEST_F(StatsAccumulatorTest, testStartStop) {
   StatsAccumulator statsAcc(mConf);

   EXPECT_FALSE(statsAcc.IsRunning());
   statsAcc.Start();
   EXPECT_TRUE(statsAcc.IsRunning());
   statsAcc.Join();
}

TEST_F(StatsAccumulatorTest, testAccumulatingStatsDefaultConfig) {
   std::string mTmpWriteLocation("/tmp/tmp.yaml");
   int pid = getpid();
   mTmpWriteLocation += boost::lexical_cast<std::string > (pid);
   remove(mTmpWriteLocation.c_str());
   SetupSendAndRcvQueues(mTmpWriteLocation);
   StatsAccumulator statsAcc(mConf);

   statsAcc.Start();
   EXPECT_TRUE(statsAcc.IsRunning());

   time_t syncTime;
   SyncWithStatsAccumulator(mStatsIntervalSeconds, syncTime);

   time_t nextExpectedTime = syncTime + mStatsIntervalSeconds;
   SendStatsToAccumulator("statA1", 1, 1);
   VerifyAccumulatedStats("statA1", 1, 1,
           nextExpectedTime, mStatsIntervalSeconds);

   //VerifyNoStatsInInterval(mStatsIntervalSeconds);

   nextExpectedTime = syncTime + (mStatsIntervalSeconds * 2);
   SendStatsToAccumulator("statA2", 1, 2);
   VerifyAccumulatedStats("statA2", 1, 2,
           nextExpectedTime, mStatsIntervalSeconds);

   nextExpectedTime = syncTime + (mStatsIntervalSeconds * 3);
   SendStatsToAccumulator("statA42", 6, 7);
   VerifyAccumulatedStats("statA42", 6, 7,
           nextExpectedTime, mStatsIntervalSeconds);

   statsAcc.Stop();

   TeardownSendAndRcvQueues();
   remove(mTmpWriteLocation.c_str());
}

TEST_F(StatsAccumulatorTest, testAccumulatingStatsConfigFromTestYaml) {
   SetupSendAndRcvQueues("resources/test.yaml");
   StatsAccumulator statsAcc(mConf);

   statsAcc.Start();
   EXPECT_TRUE(statsAcc.IsRunning());

   time_t syncTime;
   SyncWithStatsAccumulator(mStatsIntervalSeconds, syncTime);

   time_t nextExpectedTime = syncTime + mStatsIntervalSeconds;
   SendStatsToAccumulator("statB1", 1, 1);
   VerifyAccumulatedStats("statB1", 1, 1,
           nextExpectedTime, mStatsIntervalSeconds);

   nextExpectedTime = syncTime + (mStatsIntervalSeconds * 2);
   SendStatsToAccumulator("statB2", 1, 2);
   VerifyAccumulatedStats("statB2", 1, 2,
           nextExpectedTime, mStatsIntervalSeconds);

   nextExpectedTime = syncTime + (mStatsIntervalSeconds * 3);
   SendStatsToAccumulator("statB52", 4, 13);
   VerifyAccumulatedStats("statB52", 4, 13,
           nextExpectedTime, mStatsIntervalSeconds);

   //VerifyNoStatsInInterval(mStatsIntervalSeconds);

   statsAcc.Stop();

   TeardownSendAndRcvQueues();
}

