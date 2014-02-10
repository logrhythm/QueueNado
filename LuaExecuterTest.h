
#pragma once

#include "gtest/gtest.h"
#include <sys/time.h>
#include "ProcessManager.h"

class LuaExecuterTest : public ::testing::Test {
public:

   LuaExecuterTest() {

   };

   ~LuaExecuterTest() {

   }

protected:

   virtual void SetUp() {
   };

   virtual void TearDown() {

   };
    void StartTimedSection(const double expectedTimePerTransaction,const unsigned int totalTransactions) {
      t_expectedTime = expectedTimePerTransaction*totalTransactions * 1000000.0;
      t_totalTransactions = totalTransactions;
      gettimeofday(&t_startTime, NULL);
   }

   void EndTimedSection() {
      gettimeofday(&t_endTime, NULL);
      std::cout << "Expected Time: " << t_expectedTime / 1000000L << std::endl;
      t_elapsedUS = (t_endTime.tv_sec - t_startTime.tv_sec)*1000000L + (t_endTime.tv_usec - t_startTime.tv_usec);
      std::cout << std::dec << "Elapsed Time :" << t_elapsedUS / 1000000L << "." << std::setfill('0') << std::setw(6) << t_elapsedUS % 1000000 << "s" << std::endl;
      double totalTransactionsPS = (t_totalTransactions * 1.0) / (t_elapsedUS * 1.0 / 1000000.0);
      std::cout << "Transactions/second :" << totalTransactionsPS << std::endl;
      
   }

   bool TimedSectionPassed() {
      return (t_elapsedUS < t_expectedTime);
   }
   double t_expectedTime;
   time_t t_elapsedUS;
   timeval t_startTime;
   size_t t_packetSize;
   timeval t_endTime;
   unsigned int t_totalTransactions;
private:

};

