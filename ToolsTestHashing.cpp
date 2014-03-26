/**
 * 
 * Statistics for Murmur3 and City32 for hashing Msg UUIDs
 * Non failing tests. You can run this by doing 
 *  sudo ../source/MotherForker ./ToolsTest --gtest_filter=*Hash* --gtest_also_run_disabled_tests
 * 
 * Both tests takes in total approx 5 minutes to run. Most of the time 
 * is used for building up the test data. The actual hashing takes only in the seconds.
 */

#include <city.h>
#include <gtest/gtest.h>
#include <string>
#include <deque>
#include <algorithm>
#include <sstream>
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include <iostream>
#include <map>
#include <algorithm>
#include <g2log.hpp>

#include "StopWatch.h"
#include "MurMurHash.h"


namespace {
   typedef long long NumberType;

   const NumberType gNumberOfFiles = 200000; //. 2 000 000
   const NumberType gNumberOfFoldersPerPartition = 32; //16 000;
   const NumberType gNumberOfPartitions = 4; // 23;   // i.e6400000
   const NumberType gNumberOfBuckets = gNumberOfFoldersPerPartition * gNumberOfPartitions;

   const NumberType gNumberOfSessions = gNumberOfFiles * gNumberOfBuckets;
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
   // This is NOT done through the MsgUUID::GetMsgUuid(..) since we do not want
   // the std::mutex overhead 
   std::deque<std::string> CreateSessionIDs(const NumberType amount) {
      static UuidGenerator generator;
      std::deque<std::string> created{size_t(amount), {""}};
      std::generate(created.begin(), created.end(), [] {return generator.ThreadUnsafeMsgUuidCreator();});
      return created;
   }
   
   
   struct Stats {
      Stats() : maxOfHitsInBucket(0), minOfHitsInBucket(0), mean(0), stdDeviation(0) { }
      NumberType maxOfHitsInBucket;
      NumberType minOfHitsInBucket;
      NumberType mean;
      NumberType stdDeviation;
   };


   /// Used for std::accumulate in map
   NumberType AddToSum(NumberType lhs, const std::pair<NumberType, NumberType>& rhs) {
      return (lhs + rhs.second);
   }

   Stats CalculateStats(const std::map<NumberType, NumberType>& values) {
      auto sum = std::accumulate(values.begin(), values.end(), 0, AddToSum);
      auto size = values.size();
      auto mean = (sum / size);


      // std deviation http://www.social-science.co.uk/research/?n1=&id=93
      // and http://stackoverflow.com/questions/7616511/calculate-mean-and-standard-deviation-from-a-vector-of-samples-in-c-using-boos
      NumberType accumulated = 0;
      NumberType min = mean;
      NumberType max = 0;
      for (const auto& pair : values) {
         accumulated += ((pair.second - mean) * (pair.second - mean));
         min = std::min(pair.second, min);
         max = std::max(pair.second, max);
      }


      auto stdDeviation = std::sqrt(accumulated / (size - 1));

      Stats stats;
      stats.stdDeviation = stdDeviation;
      stats.minOfHitsInBucket = min;
      stats.maxOfHitsInBucket = max;
      stats.mean = mean;
      return stats;
   }

   // this will take approx 1 minute
   const std::deque<std::string> gAllSessions{CreateSessionIDs(gNumberOfSessions)};
} // anonymous

// enable by adding --gtest_also_run_disabled_tests
TEST(TestCityHash, DISABLED_CityHash32) {

   StopWatch watch;

   std::map<NumberType, NumberType> counterHashed;
   for (const auto& session : gAllSessions) {
      auto hashed = CityHash32(session.data(), session.size());
      auto bucket = hashed % gNumberOfBuckets;
      counterHashed[bucket]++;
   }
   std::ostringstream oss;
   oss << "CityHash32 Hashing : " << gNumberOfSessions << ", took: " << watch.ElapsedSec() << " seconds" << std::endl;
   oss << "CityHash32 Number of buckets: " << gNumberOfBuckets << ", number of hashed buckets: " << counterHashed.size() << std::endl;
   LOG(INFO) << oss.str();

   // Commented away but left for visibility in case anyone wanted to see or plot the exact values
   //for (auto& b : counterHashed) {
   //   LOG(DEBUG) << b.first << ": " << b.second << std::endl;
   //}
   Stats stats = CalculateStats(counterHashed);
   oss.str(""); // cleared.
   oss << "\nCityHash32\tMin in a bucket: " << stats.minOfHitsInBucket;
   oss << "\nCityHash32\tMax in a bucket: " << stats.maxOfHitsInBucket;
   oss << "\nCityHash32\tMean overall: " << stats.mean;
   oss << "\nCityHash32\tStd deviation: : " << stats.stdDeviation << std::endl;
   LOG(INFO) << oss.str();



   ASSERT_EQ(gAllSessions.size(), gNumberOfSessions);
   ASSERT_EQ(counterHashed.size(), gNumberOfBuckets);
   ASSERT_NE(stats.minOfHitsInBucket, 0);
}


//  enable by adding --gtest_also_run_disabled_tests
TEST(TestCityHash, DISABLED_Murmur3Hash) {


   std::map<NumberType, NumberType> counterHashed;
   std::deque<std::vector < uint8_t>> sessionsConverted;
   // Converted the previously generated input for Murmur3
   for (const auto& session : gAllSessions) {
      std::uint8_t* raw = reinterpret_cast<uint8_t*> (const_cast<char*> (session.data()));
      std::vector<uint8_t> input{raw, raw + session.size()};
      sessionsConverted.push_back(input);
   }

   StopWatch watch;
   for (auto& session : sessionsConverted) {
      auto hashed = MurMurHash::MurMur3_32(session);
      auto bucket = hashed % gNumberOfBuckets;
      counterHashed[bucket]++;
   }

   std::ostringstream oss;
   oss << "Murmur3Hash Hashing : " << gNumberOfSessions << ", took: " << watch.ElapsedSec() << " seconds" << std::endl;
   oss << "Murmur3Hash Number of buckets: " << gNumberOfBuckets << ", number of hashed buckets: " << counterHashed.size() << std::endl;
   LOG(INFO) << oss.str();

   // Commented away but left for visibility in case anyone wanted to see or plot the exact values
   //for (auto& b : counterHashed) {
   //LOG(DEBUG) << b.first << ": " << b.second << std::endl;
   //}
   Stats stats = CalculateStats(counterHashed);
   oss.str(""); // cleared.
   oss << "\nMurmur3Hash\tMin in a bucket: " << stats.minOfHitsInBucket;
   oss << "\nMurmur3Hash\tMax in a bucket: " << stats.maxOfHitsInBucket;
   oss << "\nMurmur3Hash\tMean overall: " << stats.mean;
   oss << "\nMurmur3Hash\tStd deviation: : " << stats.stdDeviation << std::endl;
   LOG(INFO) << oss.str();



   ASSERT_EQ(sessionsConverted.size(), gNumberOfSessions);
   ASSERT_EQ(counterHashed.size(), gNumberOfBuckets);
   ASSERT_NE(stats.minOfHitsInBucket, 0);
}
// just generate tons of stuff
//    
