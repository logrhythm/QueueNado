#include <city.h>
#include <gtest/gtest.h>
#include <string>
#include <deque>
#include <algorithm>
#include <sstream>
//#include "MsgUuid.h"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "StopWatch.h"
#include <iostream>
#include <map>
// We only want to build up this information ONCE
namespace {
   const size_t gNumberOfFiles = 2000000;
   const size_t gNumberOfFoldersPerPartition = 100; //16000;
   const size_t gNumberOfPartitions = 1; // 128;
   struct UuidGenerator {
      boost::mt19937 mRandomEngine;
      boost::uuids::basic_random_generator<boost::mt19937> mUuidGenerator;

      UuidGenerator() : mRandomEngine { }, mUuidGenerator{mRandomEngine}
      {
         mRandomEngine.seed(boost::uuids::detail::seed_rng()());
      }

      std::string ThreadUnsafeMsgUuidCreator(void) {
         boost::uuids::uuid u1;
         u1 = mUuidGenerator();
         std::stringstream ss;
         ss << u1;
         return ss.str();
      }
   };


   // Will have as many session IDs as our maximum number of files
   // totally on our system http://stackoverflow.com/questions/15220199/how-would-you-initialize-a-const-vector-of-function-results-using-c11

   std::deque<std::string> CreateSessionIDs(const size_t amount) {
      static UuidGenerator generator;
      
      std::deque<std::string> created{amount,
         {""}};
      std::generate(created.begin(), created.end(), [] {
         return generator.ThreadUnsafeMsgUuidCreator();
      });
      return created;
   }
}

TEST(TestCityHash, ThisTestWillFail) {
   const size_t numberOfSessions = 2000000; //gNumberOfFiles * gNumberOfFoldersPerPartition * gNumberOfPartitions;
   StopWatch watch;
   auto allSessions = CreateSessionIDs(numberOfSessions);
   std::cout << "Creating : " << numberOfSessions << ", took: " << watch.ElapsedSec() << " seconds" << std::endl;
   watch.Restart();
   
   std::map<uint32_t, int> counter;
   std::map<uint32_t, int> counterHashed;
   
   for(const auto& session: allSessions) {
      auto hashed = CityHash32(session.c_str(), session.size());
      counter[hashed]++;
      
      auto bucket =  hashed % gNumberOfFoldersPerPartition; // digit % size
      counterHashed[bucket]++;
   }
   std::cout << "Hashing : " << numberOfSessions << ", took: " << watch.ElapsedSec() << " seconds" << std::endl;
   std::cout << "Number of raw bins: " << counter.size() << std::endl;
   std::cout << "Number of buckets: " <<  gNumberOfFoldersPerPartition << ", number of hashed buckets: " << counterHashed.size() << std::endl;
   for(auto& b : counterHashed) {
      std::cout << b.first << ": " << b.second << std::endl;
   }
          
   // left to do,. calculate statistical variance betwen all the maps.
   ASSERT_EQ(allSessions.size(), numberOfSessions);
}

// just generate tons of stuff
// 