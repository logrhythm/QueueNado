#pragma once


#include "gtest/gtest.h"
#include "Rifle.h"
#include "Vampire.h"

#include <atomic>
#include <sys/time.h>
#include "czmq.h"

class RifleVampireTests : public ::testing::Test {
public:

   RifleVampireTests() {
   };
   int mPid;
   std::string mIpcLocation;
   std::string mTcpLocation;
   std::string mInprocLocation;
   static std::atomic<int> mShotsDeleted;
   static std::string GetTcpLocation();
   static std::string GetIpcLocation();
   static std::string GetInprocLocation();
   void RifleThread(int numberOfMessages, std::string& location,
           std::string& exampleData, int hwm, int ioThreads, bool ownSocket);
   void ShootStakeThread(int numberOfMessages,
           std::string& binding, std::vector<std::pair<void*, unsigned int> >& exampleData, 
           bool ownSocket);
   void VampireThread(int numberOfMessages, std::string& location,
         std::string& exampleData, int hwm, int ioThreads, bool ownSocket);
   void StakeAVampireThread(int numberOfMessages,
           std::string& location, std::vector<std::pair<void*, unsigned int> >& exampleData, 
           int hwm, int ioThreads);
   void ShootZeroCopyThread(int numberOfMessages,
        std::string& location, std::string& exampleData,int hwm, int ioThreads, 
        bool ownSocket);
   void OneRifleNVampiresBenchmark(int nVampires, int nIOThreads,
           int rifleHWM, int vampireHWM, std::string& location, int dataSize,
           int nShotsPerVampire, int expectedSpeed);
   void OneRifleNVampiresStakeBenchmark(int nVampires, int nIOThreads,
           int rifleHWM, int vampireHWM, std::string& location, int dataSize,
           int nShotsPerVampire, int expectedSpeed);
   void NRiflesOneVampireBenchmark(int nRifles, int nIOThreads,
           int rifleHWM, int vampireHWM, std::string& location, int dataSize,
           int nShotsPerRifle, int expectedSpeed);
   void NRiflesOneVampireBenchmarkZeroCopy(int nRifles, int nIOThreads,
           int rifleHWM, int vampireHWM, std::string& location, int dataSize,
           int nShotsPerRifle, int expectedSpeed);

   class RifleAmmo {
   public:

      RifleAmmo() :
      count(1), location() {
      }
      int count;
      std::string location;
   };

protected:

   virtual void SetUp() {
   };

   virtual void TearDown() {
   };

   void MakeBigData(size_t size) {
      t_bigDataSize = size - (size % 4);
      if (t_bigData)
         delete t_bigData;
      t_bigData = new u_char[t_bigDataSize];
      for (unsigned int i = 0; i < t_bigDataSize / 4; i++) {
         long data = random();
         t_bigData[i * 4] = (data >> 24) & 0xff;
         t_bigData[i * 4 + 1] = (data >> 16) & 0xff;
         t_bigData[i * 4 + 2] = (data >> 8) & 0xff;
         t_bigData[i * 4 + 3] = (data) & 0xff;
      }
   }

   void MakePacketsData() {
      long bigNumber = 1500;
      t_bigDataSize = bigNumber - (bigNumber % 4);
      if (t_bigData)
         delete t_bigData;
      t_bigData = new u_char[t_bigDataSize];
      for (unsigned int i = 0; i < t_bigDataSize / 4; i++) {
         long data = random();
         t_bigData[i * 4] = (data >> 24) & 0xff;
         t_bigData[i * 4 + 1] = (data >> 16) & 0xff;
         t_bigData[i * 4 + 2] = (data >> 8) & 0xff;
         t_bigData[i * 4 + 3] = (data) & 0xff;
      }
   }

   void SetExpectedTime(size_t numberOfPackets, size_t packetSize, size_t RateInMbps, size_t transactionsPerSecond) {
      size_t totalBytes = (numberOfPackets * packetSize);
      t_totalTransactions = numberOfPackets;
      t_packetSize = packetSize;
      t_targetTransactionsPerSecond = transactionsPerSecond;
      std::cout << std::dec << "Total MBytes: " << totalBytes / (1024 * 1024) << std::endl;
      t_expectedTime = ((totalBytes * 1.0) / (RateInMbps / 8.0));

      std::cout << "Expected Time :" << t_expectedTime / 1000000.0 << "s at " << RateInMbps << "Mbps" << std::endl;
   }

   void StartTimedSection() {
      gettimeofday(&t_startTime, NULL);
   }

   void EndTimedSection() {
      gettimeofday(&t_endTime, NULL);
      t_elapsedUS = (t_endTime.tv_sec - t_startTime.tv_sec)*1000000L + (t_endTime.tv_usec - t_startTime.tv_usec);
      std::cout << std::dec << "Elapsed Time :" << t_elapsedUS / 1000000L << "." << std::setfill('0') << std::setw(6) << t_elapsedUS % 1000000 << "s" << std::endl;
      double totalTransactionsPS = (t_totalTransactions * 1.0) / (t_elapsedUS * 1.0 / 1000000.0);
      std::cout << "Transactions/second :" << totalTransactionsPS << " at ";
      double totalSpeed = (t_packetSize) * totalTransactionsPS / (125000.0);
      std::cout << totalSpeed << "Mbps" << std::endl;
   }

   bool TimedSectionPassed() {
      return (t_elapsedUS < t_expectedTime);
   }
   int mPacketsSeen;
   int mPacketsToTest;
   static int gCurrentPacketSize;
   int t_totalTransactions;
   int t_targetTransactionsPerSecond;
   u_char* t_bigData;
   size_t t_bigDataSize;
   double t_expectedTime;
   time_t t_elapsedUS;
   timeval t_startTime;
   size_t t_packetSize;
   timeval t_endTime;
private:

};
