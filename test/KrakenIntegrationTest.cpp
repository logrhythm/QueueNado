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
#include <g3log/g3log.hpp>
#include "KrakenIntegrationHelper.h"

using namespace KrakenIntegrationHelper;




// This integration test is important since it's the only one that truly tests
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
   ASSERT_EQ(zctx_interrupted, false);



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

   std::shared_ptr<std::atomic<size_t>> expectedToReceive {new std::atomic<size_t>{0}};
   std::shared_ptr<Harpoon> harpoon = std::make_shared<Harpoon>();
   auto spearResult = harpoon->Aim(queue);
   ASSERT_EQ(spearResult, Harpoon::Spear::IMPALED);
   harpoon->MaxWaitInMs(1000);

   // RECEIVER
   using HarpoonReceived = std::vector<Kraken::Chunks>;
   std::future<HarpoonReceived> willReceive = std::async(std::launch::async,  [expectedToReceive, harpoon] {
      StopWatch stopWatch;
      HarpoonReceived data;
      size_t counter = 0;
      while (expectedToReceive->load() == 0 && stopWatch.ElapsedSec() < 10) {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }


      size_t expected = expectedToReceive->load();
      while (data.size() != expected  && stopWatch.ElapsedSec() < 10) {
         Kraken::Chunks blood;
         auto harpoonResult = harpoon->Heave(blood);
         ++counter;
         EXPECT_TRUE(Harpoon::Battling::CONTINUE == harpoonResult)
               << harpoon->EnumToString(harpoonResult) << ", counter: " << counter << "/" << expectedToReceive->load() << " received: " << data.size() << "/" << expected;

         if (blood.size() > 0) {
            data.push_back(blood);
         } else {
            ADD_FAILURE() << "counter: " << counter;
            return data;
         }
      }

      // Receive final breach
      Kraken::Chunks blood;
      auto harpoonResult = harpoon->Heave(blood);
      EXPECT_EQ(harpoonResult, Harpoon::Battling::VICTORIOUS) << harpoon->EnumToString(harpoonResult);
      EXPECT_EQ(blood.size(), 0);
      return data;
   });



   // SENDER
   // send all stuff - happy path
   const std::string noError = {"no error"};
   const size_t kExpectedNumberOfSendActions = 6;

   // signal the async job to exit before the timer if all the
   // expected data is received
   expectedToReceive->store(kExpectedNumberOfSendActions);

   // 0. send header
   auto status = KrakenBattle::ForwardChunksToClient(&kraken, session, header, SendType::Begin, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);

   // 1. send data1: world
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, chunk1, SendType::Data, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);

   // 2. send data2: 10MB - which will be split up multiple times
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, chunk2, SendType::Data, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);

   // 3. send data3: done
   const Kraken::Chunks noChunk = {};
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, noChunk, SendType::Done, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);

   // 4. send data4: end
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
   const auto kReceived = willReceive.get();
   const auto kChunkHeader = header;
   const auto kExpectedHeader = MergeData(session, SendType::Begin, kChunkHeader, noError);
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




// This integration test is important also since it's the only one that truly tests
// the KrakenBattle::SendChunks (cpp hidden file)
//
// To verify the expected steps in this communication
// 0. some header (in this case just 'hello')
// 1. uuid<DATA>w,o,r,l,d
// 2. uuid<DATA> - part1 10MB
// 3. uuid<DATA> - part2 10MB
// ---> ABORT sent from the receiver/Harpoon
// 4. uuid<DONE>
// 5. uuid<END>
TEST_F(KrakenIntegrationTest, AbortedCommunication) {
   using namespace KrakenBattle;
   ASSERT_EQ(zctx_interrupted, false);



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

   std::shared_ptr<std::atomic<size_t>> expectedToReceive {new std::atomic<size_t>{0}};
   std::shared_ptr<Harpoon> harpoon = std::make_shared<Harpoon>();
   auto spearResult = harpoon->Aim(queue);
   ASSERT_EQ(spearResult, Harpoon::Spear::IMPALED);
   harpoon->MaxWaitInMs(1000);

   // RECEIVER
   using HarpoonReceived = std::vector<Kraken::Chunks>;
   std::future<HarpoonReceived> willReceive = std::async(std::launch::async,  [expectedToReceive, harpoon] {
      StopWatch stopWatch;
      HarpoonReceived data;
      size_t counter = 0;
      // i.e. After the 3rd is received but before dealing with the 4th we abort
      // See also comment above this test
      size_t abortAt = 3; 
      bool cancelWasCalled = {false};
      while (expectedToReceive->load() == 0 && stopWatch.ElapsedSec() < 10) {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }


      size_t expected = expectedToReceive->load();
      while (data.size() != expected  && stopWatch.ElapsedSec() < 10 && !zctx_interrupted) {
         auto harpoonResult = Harpoon::Battling::CONTINUE;
         if (abortAt == counter) {
            LOG(INFO) << "#counter: " << counter << ", sending cancel through harpoon back to kraken";
            harpoonResult = harpoon->Cancel();
            cancelWasCalled = true;
            EXPECT_TRUE(Harpoon::Battling::CONTINUE == harpoonResult);
            break;
         } else {
            Kraken::Chunks blood;
            harpoonResult  = harpoon->Heave(blood);
            EXPECT_TRUE(Harpoon::Battling::CONTINUE == harpoonResult)
                  << harpoon->EnumToString(harpoonResult) << ", counter: " << counter << "/" << expectedToReceive->load() << " received: " << data.size() << "/" << expected;

            if (blood.size() > 0) {
               data.push_back(blood);
            } else {
               ADD_FAILURE() << "counter: " << counter;
               return data;
            }
         }
         ++counter;
      }


      EXPECT_TRUE(cancelWasCalled);
      EXPECT_EQ(3, counter); // aborted the loop after cancel
      // Once we have called cancel we should not expect anything else. We can attempt 
      // but at this point communication is dead and timeout is what to be expected
      Kraken::Chunks blood;
      auto harpoonResult = harpoon->Heave(blood);
      EXPECT_EQ(harpoonResult, Harpoon::Battling::TIMEOUT) << harpoon->EnumToString(harpoonResult);
      EXPECT_EQ(blood.size(), 0);
      return data;
   });



   // SENDER
   // send all stuff - happy path
   const std::string noError = {"no error"};
   const size_t kActualExpectedNumberOfSendActions = 3;
   const size_t kAttemptToDoSendActions = 6; // it should have been 6 but after 3 it got aborted

   // signal the async job to exit before the timer if all the
   // expected data is received
   expectedToReceive->store(kAttemptToDoSendActions);

   // 0. send header
   auto status = KrakenBattle::ForwardChunksToClient(&kraken, session, header, SendType::Begin, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);
   
   // 1. send data1: world
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, chunk1, SendType::Data, noError);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);

   // 2. send data2: 10MB - which will be split up multiple times
   LOG(INFO) << "1" << __FUNCTION__;
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, chunk2, SendType::Data, noError);
   LOG(INFO) << KrakenBattle::EnumToString(status);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Continue) << "received: " << KrakenBattle::EnumToString(status);

   // 3. send data3: Stop will be received for this and anything after this.
   LOG(INFO) << "2" << __FUNCTION__;
   const Kraken::Chunks noChunk = {};
   status = KrakenBattle::ForwardChunksToClient(&kraken, session, noChunk, SendType::End, noError);
   LOG(INFO) << KrakenBattle::EnumToString(status);
   EXPECT_EQ(status, KrakenBattle::ProgressType::Stop) << "received: " << KrakenBattle::EnumToString(status);



   // VERIFY
   // expected
   // 0. header: hello
   // 1. uuid<DATA>world
   // 2. uuid<DATA> - part1 10MB
   // 3. uuid<DATA> - part2 10MB
   // 4. uuid<DONE>
   // 5. uuid<END>
   const auto kReceived = willReceive.get();
   const auto kChunkHeader = header;
   const auto kExpectedHeader = MergeData(session, SendType::Begin, kChunkHeader, noError);
   const auto kSessionHeader = MergeData(session, SendType::Data, {}, {});
   auto chunk1Expected = MergeData(session, SendType::Data, chunk1, noError);

   // 1
   ASSERT_EQ(kReceived.size(), kActualExpectedNumberOfSendActions);
   auto vec = kReceived[0];
   EXPECT_TRUE((kExpectedHeader == vec)) << vectorToString(vec);

   vec = kReceived[1];
   EXPECT_TRUE((chunk1Expected == vec)) << vectorToString(vec);

   size_t mcount = 0;
   LOG(INFO) << ++mcount << "************************";
   vec = kReceived[2];
   Kraken::Chunks chunk2_part1 = {};
   chunk2_part1.reserve(kMaxChunkSize_10MB);
   std::copy(kSessionHeader.begin(), kSessionHeader.end(), std::back_inserter(chunk2_part1));
   std::copy(chunk2.begin(), chunk2.begin() + kMaxChunkSize_10MB - kSessionHeader.size(),
             std::back_inserter(chunk2_part1));
   EXPECT_EQ(chunk2_part1.size(), kMaxChunkSize_10MB);
   EXPECT_TRUE((chunk2_part1 == vec)) << "vec50 = " << vectorToString(vec, 50) << "\n\nchunk2_part1_50 = " << vectorToString(chunk2_part1, 50);

}

