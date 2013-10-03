#pragma once

#include <sys/time.h>

#include "gtest/gtest.h"
#include "boost/lexical_cast.hpp"
#include <csignal>

class DiskPacketCaptureTest : public ::testing::Test {
public:

   DiskPacketCaptureTest() {
   };
protected:

    virtual void SetUp() {
      testDir << "/tmp/DiskPacketCaptureTest";
 
     std::string makeADir;
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      system(makeADir.c_str());

      makeADir = "mkdir -p ";
      makeADir += testDir.str();
      system(makeADir.c_str());
    };
 
    virtual void TearDown() {

      std::string makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
    };

   void StartTimedSection(const double expectedTimePerTransaction, const unsigned int totalTransactions) {
      t_expectedTime = expectedTimePerTransaction * totalTransactions * 1000000.0;
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
   std::stringstream testDir;
};



