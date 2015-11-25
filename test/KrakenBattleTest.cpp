/*
 * File:   KrakenBattleTest.cpp
 * Author: Kjell Hedstrom
 *
 * Created on February 9, 2015
 */


#include "KrakenBattleTest.h"
#include "Kraken.h"
#include "KrakenBattle.h"
#include "Harpoon.h"


namespace {
   const std::string gUuid =  "734a83c7-9435-4605-b1f9-4724c81faf21";
   const std::string gEmptyUuid = "00000000-0000-0000-0000-000000000000";
   const Kraken::Chunks gEmptyData = {};
   const Kraken::Chunks gData = {1, 2, 128, 'k', 'j', '2', '1'};

   std::string vectorToString(const std::vector<uint8_t>& vec) {
      std::string data;
      std::copy(vec.begin(), vec.end(), std::back_inserter(data));
      return data;
   }
}


TEST_F(KrakenBattleTest, MergeDataTypes_ErrorType) {
   auto failed = MergeData("123", KrakenBattle::SendType::Error, gEmptyData, "bad stuff");
   std::vector<uint8_t> expected {'1', '2', '3', '<', 'E', 'R', 'R', 'O', 'R', '>', 'b', 'a', 'd', ' ', 's', 't', 'u', 'f', 'f'};

   EXPECT_TRUE((failed == expected)) << "\nfailed: [" << vectorToString(failed) << "]\nexpected: [" << vectorToString(expected) << "]";
}

TEST_F(KrakenBattleTest, MergeDataTypes_DataType) {
   auto send = MergeData("321", KrakenBattle::SendType::Data, gData, "ignored");
   std::vector<uint8_t> expected {'3', '2', '1', '<', 'D', 'A', 'T', 'A', '>', 1, 2, 128, 'k', 'j', '2', '1'};

   EXPECT_TRUE((send == expected)) << "\nfailed: [" << vectorToString(send) << "]\nexpected: [" << vectorToString(expected) << "]";
}

TEST_F(KrakenBattleTest, MergeDataTypes_DoneType) {
   auto send = MergeData(gUuid, KrakenBattle::SendType::Done, gData, "ignored");
   std::string sendStatus = "<DONE>";

   std::vector<uint8_t> expected;
   std::copy(gUuid.begin(), gUuid.end(), std::back_inserter(expected));
   std::copy(sendStatus.begin(), sendStatus.end(), std::back_inserter(expected));


   EXPECT_TRUE((send == expected)) << "\nfailed: [" << vectorToString(send) << "]\nexpected: [" << vectorToString(expected) << "]";
}


TEST_F(KrakenBattleTest, MergeDataTypes_EndType) {
   auto send = MergeData("ignored_uuid", KrakenBattle::SendType::End, gData, "ignored");
   std::string sendStatus = "<END>";

   std::vector<uint8_t> expected;
   std::copy(gEmptyUuid.begin(), gEmptyUuid.end(), std::back_inserter(expected));
   std::copy(sendStatus.begin(), sendStatus.end(), std::back_inserter(expected));

   EXPECT_TRUE((send == expected)) << "\nfailed: [" << vectorToString(send) << "]\nexpected: [" << vectorToString(expected) << "]";
}


