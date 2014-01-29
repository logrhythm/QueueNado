#pragma once

#include "MockDiskCleanup.h"
#include "MockConfSlave.h"
#include "gtest/gtest.h"
#include "boost/lexical_cast.hpp"
#include <csignal>
#include <sstream>
#include <sys/time.h>
#include <g2log.hpp>
#include "ProcessManager.h"
class DiskCleanupTest : public ::testing::Test {
public:

   DiskCleanupTest() : bogusTime(123456), totalHits(0) {

   };
protected:

   virtual void SetUp() {
      memset(reinterpret_cast<void*>(&stats),0,sizeof(stats));
      stats.currentTime = std::time(NULL);
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id0", "id0"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id1", "id1"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id2", "id2"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id3", "id3"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id4", "id4"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id5", "id5"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id6", "id6"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id7", "id7"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id8", "id8"));
      bogusFileList.insert(std::make_tuple<std::string, std::string>("/path/id9", "id9"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "1"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "2"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "3"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "4"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "5"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "6"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "7"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "8"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "9"));
      bogusIdsAndIndex.push_back(std::make_pair<std::string, std::string>("abc", "10"));
      
      
      testDir << "/tmp/TooMuchPcap";

      std::string makeADir;
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      system(makeADir.c_str());

      makeADir = "mkdir -p ";
      makeADir += testDir.str();
      auto result = system(makeADir.c_str());
      LOG_IF(WARNING, 0 != result) << "Could not create directory with command: " << makeADir;
      totalHits = 0;
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
   std::stringstream testDir;
   PathAndFileNames bogusFileList;
   IdsAndIndexes bogusIdsAndIndex;
   time_t bogusTime;
   DiskCleanup::StatInfo stats;
   size_t totalHits;
private:

};



