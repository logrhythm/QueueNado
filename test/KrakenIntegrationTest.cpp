/*
 * File:   KrakenIntegrationTest.cpp
 * Author: Kjell Hedstrom
 *
 * Created on February 9, 2015
 */


#include "KrakenIntegrationTest.h"
#include "Kraken.h"
#include "KrakenBattle.h"
#include "Harpoon.h"
#include <memory>
#include <atomic>
#include <thread>
#include <future>
#include <StopWatch.h>
#include <cstdlib>

namespace {
   Kraken::Chunks GetRandomData(const size_t sizeOfData) {
      Kraken::Chunks someData;
      for (size_t j = 0; j < sizeOfData; j++) {
         someData.push_back(rand() % 255);
      }
      return someData;
   }


   std::string vectorToString(const std::vector<uint8_t>& vec) {
      std::string data;
      std::copy(vec.begin(), vec.end(), std::back_inserter(data));
      return data;
   }

   std::string vectorToString(const std::vector<uint8_t>& vec, size_t stopper) {
      std::string data;
      stopper = std::min(stopper, vec.size());
      std::copy(vec.begin(), vec.begin() + stopper, std::back_inserter(data));
      return data;
   }

} // anonymous




// This integration test is imporant also since it's the only one that truly tests
// the KrakenBattle::SendChunks (cpp hidden file)
//
// To verify the expected steps in this communication
// 0. some header (in this case just 'hello')
// 1. uuid<DATA>w,o,r,l,d
// 2. uuid<DATA> - part1 10MB
// 3. uuid<DATA> - part2 10MB
// 4. uuid<DONE>
// 5. uuid<END>
TEST_F(KrakenIntegrationTest, VerifyCommunication) {
   using namespace KrakenBattle;

   Kraken::Chunks header = {'h', 'e', 'l', 'l', 'o'};

// 10MB
   const size_t kMaxChunkSize_10MB = 10 * 1024 * 1024;
   const std::string session = "some-random-session";
   const auto chunk1 = Kraken::Chunks{'w', 'o', 'r', 'l', 'd'};
   const auto chunk2 = GetRandomData(kMaxChunkSize_10MB);



   const std::string queue = "tcp://127.0.0.1:15123";
   Kraken kraken;
   kraken.MaxWaitInMs(1000);
   auto spear = kraken.SetLocation(queue);
   EXPECT_EQ(spear, Kraken::Spear::IMPALED) << "spear: " << static_cast<int>(spear);

   size_t tooHighValue = 100;
   std::shared_ptr<std::atomic<size_t>> allReceived {new std::atomic<size_t>{tooHighValue}};
   std::shared_ptr<Harpoon> harpoon = std::make_shared<Harpoon>();
   harpoon->Aim(queue);
   harpoon->MaxWaitInMs(1000);

   // RECEIVER
   using HarpoonReceived = std::vector<Kraken::Chunks>;
   std::future<HarpoonReceived> willReceive = std::async(std::launch::async,  [allReceived, harpoon] {
      StopWatch stopWatch;
      HarpoonReceived data;
      while (allReceived->load() != data.size() && stopWatch.ElapsedSec() < 10) {
         Kraken::Chunks blood;
         harpoon->Heave(blood);
         if (blood.size() > 0) {
            data.push_back(blood);
         }
      }
      return data;
   });



   // SENDER
   // send all stuff - happy path
   const std::string noError = {"no error"};
   // send header
   auto status = KrakenBattle::ForwardChunksToClient(&kraken, session, header, SendType::Data, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);

   // send data1: world
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, chunk1, SendType::Data, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue)<< "received: " << KrakenBattle::EnumToString(status);

   // send data2: 10MB - which will be split up multiple times
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, chunk2, SendType::Data, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue)<< "received: " << KrakenBattle::EnumToString(status);

   // send data3: done
   const Kraken::Chunks noChunk = {};
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, noChunk, SendType::Done, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);
   // send data4: end
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, noChunk, SendType::End, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);


   

   // VERIFY
   // expected
   // 0. header: hello
   // 1. uuid<DATA>world
   // 2. uuid<DATA> - part1 10MB
   // 3. uuid<DATA> - part2 10MB
   // 4. uuid<DONE>
   // 5. uuid<END>
   const size_t kExpectedNumberOfSendActions = 6;
   allReceived->store(kExpectedNumberOfSendActions); // signal the async job to exit before the timer
   const auto kReceived = willReceive.get();
   const auto kChunkHeader = header;
   const auto kExpectedHeader = MergeData(session, SendType::Data, kChunkHeader, noError);
   const auto kSessionHeader = MergeData(session, SendType::Data, {}, {});
   auto chunk1Expected = MergeData(session, SendType::Data, chunk1, noError);

   ASSERT_EQ(kReceived.size(), kExpectedNumberOfSendActions);
   auto vec = kReceived[0];
   EXPECT_TRUE((kExpectedHeader == vec)) << vectorToString(vec);

   vec = kReceived[1];
   EXPECT_TRUE((chunk1Expected == vec)) << vectorToString(vec);

   vec = kReceived[2];
   Kraken::Chunks chunk2_part1 = {};
   chunk2_part1.reserve(kMaxChunkSize_10MB);
   std::copy(kSessionHeader.begin(), kSessionHeader.end(), std::back_inserter(chunk2_part1));
   std::copy(chunk2.begin(), chunk2.begin() + kMaxChunkSize_10MB - kSessionHeader.size(),
             std::back_inserter(chunk2_part1));
   EXPECT_EQ(chunk2_part1.size(), kMaxChunkSize_10MB);
   EXPECT_TRUE((chunk2_part1 == vec)) << "vec50 = " << vectorToString(vec, 50) << "\n\nchunk2_part1_50 = " << vectorToString(chunk2_part1, 50);


   vec = kReceived[3];
   Kraken::Chunks chunk2_part2 = {};
   chunk2_part2.reserve(kMaxChunkSize_10MB);
   std::copy(kSessionHeader.begin(), kSessionHeader.end(), std::back_inserter(chunk2_part2));
   std::copy(chunk2.begin() + (kMaxChunkSize_10MB - kSessionHeader.size()), chunk2.end(), std::back_inserter(chunk2_part2));
   EXPECT_TRUE(chunk2_part2.size() < kMaxChunkSize_10MB) << "chunkSize: " << chunk2_part2.size();
   EXPECT_TRUE((chunk2_part2 == vec)) << "vec50 = " << vectorToString(vec, 50) << "\n\nchunk2_part2_50 = " << vectorToString(chunk2_part2, 50);


   vec = kReceived[4];
   auto doneExpected = MergeData(session, SendType::Done, noChunk, noError);
   EXPECT_TRUE((doneExpected == vec)) << vectorToString(vec);

   vec = kReceived[5];
   auto endExpected = MergeData(session, SendType::End, noChunk, noError);
   EXPECT_TRUE((endExpected == vec)) << vectorToString(vec);
   
}


