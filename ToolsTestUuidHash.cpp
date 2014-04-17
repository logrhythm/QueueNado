/* 
 * File:   ToolsTestUuidHash.cpp
 * Author: kjell
 *
 * Created on March 27, 2014, 2:44 PM
 */

#include <gtest/gtest.h>
#include <city.h>
#include <exception>

#include "MsgUuid.h"
#include "UuidHash.h"
#include "Death.h"

namespace {
   auto& gGenerator = networkMonitor::MsgUuid::Instance();
}




TEST(TestUuidHash, VerifyGetUuidBucketVsCityHash) {
   auto uuid = gGenerator.GetMsgUuid();
   
   size_t directoriesPerPartition = 16000;
   size_t numberOfPartitions = 1;
   
   auto hashed = CityHash32(uuid.data(), uuid.size());
   auto cityBucket = hashed % (directoriesPerPartition * numberOfPartitions);
   auto uuidBucket = UuidHash::GetUuidBucket(uuid, directoriesPerPartition, numberOfPartitions);
   EXPECT_EQ(cityBucket, uuidBucket);   
}

TEST(TestUuidHash, GetUuidBucketDirectoryIndex) {
   size_t bucket = 0;
   const size_t bucketsPerPartition = 16000;
   size_t numberOfPartitions = 1;
   size_t directory = UuidHash::GetUuidBucketDirectoryIndex(bucket, bucketsPerPartition);
   EXPECT_EQ(0, directory);

   // /pcap0   bucket 0-15999
   for (bucket = 0; bucket < bucketsPerPartition; ++bucket) {
      directory = UuidHash::GetUuidBucketDirectoryIndex(bucket, bucketsPerPartition);
      EXPECT_EQ(bucket, directory);
   }

   // pcap1 bucket 0 - 15999
   for (bucket = 16000; bucket < (2 * bucketsPerPartition); ++bucket) {
         directory = UuidHash::GetUuidBucketDirectoryIndex(bucket, bucketsPerPartition);
         EXPECT_EQ(bucket - 16000, directory) << bucket;
      }
}
   
TEST(TestUuidHash, GetUuidPartition_CorrectEnd) {
   size_t ignored1  = 1;
   size_t ignored2  = 2;
   
   auto partition = UuidHash::GetUuidPartition(ignored1, ignored2, {{"/pcap"}});
   EXPECT_EQ("/pcap/", partition);
   partition = UuidHash::GetUuidPartition(ignored1, ignored2, {{"/pcap/"}});
   EXPECT_EQ("/pcap/", partition);
   partition = UuidHash::GetUuidPartition(ignored1, ignored2, {{""}});
   EXPECT_EQ("", partition); // nothing there. do not add stuff to put it in root
}



TEST(TestUuidHash, VerifyGetUuidPartitionForOne) {
   auto uuid = gGenerator.GetMsgUuid();
   
   size_t directoriesPerPartition = 1;
   size_t numberOfPartitions = 1;
   std::vector<std::string> locations{"/pcap0"};
   
   auto uuidBucket = UuidHash::GetUuidBucket(uuid, directoriesPerPartition, numberOfPartitions);
   auto directoryIndex = UuidHash::GetUuidBucketDirectoryIndex(uuidBucket, directoriesPerPartition);   
   auto partition = UuidHash::GetUuidPartition(uuidBucket, directoriesPerPartition, locations);
   EXPECT_TRUE(uuidBucket < 16000) << uuidBucket;
   EXPECT_EQ(directoryIndex, 0);   
   EXPECT_EQ(partition, "/pcap0/");   
}



TEST(TestUuidHash, VerifyGetUuidPartitionForOneWith16000SubDirectories) {
   auto uuid = gGenerator.GetMsgUuid();
   
   size_t directoriesPerPartition = 16000;
   size_t numberOfPartitions = 1;
   std::vector<std::string> locations{"/pcap0"};
   
   auto uuidBucket = UuidHash::GetUuidBucket(uuid, directoriesPerPartition, numberOfPartitions);
   auto directoryIndex = UuidHash::GetUuidBucketDirectoryIndex(uuidBucket, directoriesPerPartition);   
   auto partition = UuidHash::GetUuidPartition(uuidBucket, directoriesPerPartition, locations);
   EXPECT_TRUE(uuidBucket < 16000) << uuidBucket;
   EXPECT_TRUE(directoryIndex < 16000);   
   EXPECT_EQ(partition, "/pcap0/");   
}

// only sub directory .../0/ exist
TEST(TestUuidHash, VerifyGetUuidPartitionForTwo) {
   std::string uuidA={"553c367a-f638-457c-9916-624e189702ef"};
   std::string uuidB={"aa681de2-d32b-4aa4-abe0-5b57d47da5de"};
   
   size_t directoriesPerPartition = 1;
   size_t numberOfPartitions = 2;
   std::vector<std::string> locations{{"/pcap0"}, {"/pcap1"}};   
   
   auto bucketA = UuidHash::GetUuidBucket(uuidA, directoriesPerPartition, numberOfPartitions);
   auto directoryIndexA = UuidHash::GetUuidBucketDirectoryIndex(bucketA, directoriesPerPartition);   
   auto partitionA = UuidHash::GetUuidPartition(bucketA, directoriesPerPartition, locations);
   EXPECT_EQ(bucketA, 0);   
   EXPECT_EQ(directoryIndexA, 0);   
   EXPECT_EQ(partitionA, "/pcap0/");   
 
   
   auto bucketB = UuidHash::GetUuidBucket(uuidB, directoriesPerPartition, numberOfPartitions); 
   auto directoryIndexB = UuidHash::GetUuidBucketDirectoryIndex(bucketB, directoriesPerPartition);   
   auto partitionB = UuidHash::GetUuidPartition(bucketB, directoriesPerPartition, locations);
   EXPECT_EQ(bucketB, 1);   
   EXPECT_EQ(directoryIndexB, 0);   
   EXPECT_EQ(partitionB, "/pcap1/");   
}

TEST(TestUuidHash, VerifyGetUuidPartitionFor16000) {
   std::string uuidA={"fffecd5f-2f85-413a-b694-408f380a3c42"};
   std::string uuidB={"161122fd-6681-42a3-b953-48beb5247172"};
   
   size_t directoriesPerPartition = 2000;
   size_t numberOfPartitions = 2;
   std::vector<std::string> locations{{"/pcap0"}, {"/pcap1"}};
   
   auto bucketA = UuidHash::GetUuidBucket(uuidA, directoriesPerPartition, numberOfPartitions);
   auto directoryIndexA = UuidHash::GetUuidBucketDirectoryIndex(bucketA, directoriesPerPartition);   
   auto partitionA = UuidHash::GetUuidPartition(bucketA, directoriesPerPartition, locations);
   EXPECT_EQ(bucketA, 1120);   
   EXPECT_EQ(directoryIndexA, 1120);   
   EXPECT_EQ(partitionA, "/pcap0/");   

   auto bucketB = UuidHash::GetUuidBucket(uuidB, directoriesPerPartition, numberOfPartitions); 
   auto directoryIndexB = UuidHash::GetUuidBucketDirectoryIndex(bucketB, directoriesPerPartition);   
   auto partitionB = UuidHash::GetUuidPartition(bucketB, directoriesPerPartition, locations);
   EXPECT_EQ(bucketB, 2854);   
   EXPECT_EQ(directoryIndexB, 854);   
   EXPECT_EQ(partitionB, "/pcap1/");   
}




// This should never happen of course
TEST(TestUuidHash, DeathForIllegalBucket) {
   std::vector<std::string> noPartitions;
   RaiiDeathCleanup cleanup;

   Death::SetupExitHandler();
   EXPECT_FALSE(Death::WasKilled());
   std::string partition;
   size_t bucket = 1;
   size_t bucketsPerPartition = 16000;
   EXPECT_NO_THROW( partition = UuidHash::GetUuidPartition(bucket, bucketsPerPartition, noPartitions));
   EXPECT_TRUE(partition.empty());
   EXPECT_TRUE(Death::WasKilled());
   
   Death::ClearExits();
   EXPECT_FALSE(Death::WasKilled());
   bucket = 32000;
   EXPECT_NO_THROW( partition = UuidHash::GetUuidPartition(bucket, bucketsPerPartition, {"one partition"}));
   EXPECT_TRUE(Death::WasKilled());
   EXPECT_TRUE(partition.empty());
       
   Death::ClearExits();
   EXPECT_FALSE(Death::WasKilled());
   bucket = 32000;  // 0 : 0-15999, 1: 16000 - 31999, 2: 32000-63000
   EXPECT_NO_THROW( partition = UuidHash::GetUuidPartition(bucket, bucketsPerPartition, {{"0"}, {"1"}, {"2"}}));
   EXPECT_FALSE(Death::WasKilled());
   EXPECT_EQ(partition, "2/");

}
