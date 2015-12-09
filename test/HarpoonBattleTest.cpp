/*
 * File:   HarpoonBattleTest.cpp
 * Author: Kjell Hedstrom
 *
 * Created on November 30, 2015
 */


#include "HarpoonBattleTest.h"
#include "Kraken.h"
#include "KrakenBattle.h"
#include "HarpoonBattle.h"
#include "KrakenIntegrationHelper.h"
#include <Death.h>
#include <memory>
#include <tuple>
#include <utility>


TEST_F(HarpoonBattleTest, EnumToString) {
   auto data = HarpoonBattle::EnumToString(HarpoonBattle::ReceivedType::Data);
   auto done = HarpoonBattle::EnumToString(HarpoonBattle::ReceivedType::Done);
   auto error = HarpoonBattle::EnumToString(HarpoonBattle::ReceivedType::Error);
   auto end = HarpoonBattle::EnumToString(HarpoonBattle::ReceivedType::End);

   EXPECT_TRUE(data == "<DATA>");
   EXPECT_TRUE(done == "<DONE>");
   EXPECT_TRUE(error == "<ERROR>");
   EXPECT_TRUE(end == "<END>");

}

TEST_F(HarpoonBattleTest, EnumToStringConverted) {
   auto data = HarpoonBattle::EnumToString(static_cast<HarpoonBattle::ReceivedType>(KrakenBattle::SendType::Data));
   auto done = HarpoonBattle::EnumToString(static_cast<HarpoonBattle::ReceivedType>(KrakenBattle::SendType::Done));
   auto error = HarpoonBattle::EnumToString(static_cast<HarpoonBattle::ReceivedType>(KrakenBattle::SendType::Error));
   auto end = HarpoonBattle::EnumToString(static_cast<HarpoonBattle::ReceivedType>(KrakenBattle::SendType::End));

   EXPECT_TRUE(data == "<DATA>");
   EXPECT_TRUE(done == "<DONE>");
   EXPECT_TRUE(error == "<ERROR>");
   EXPECT_TRUE(end == "<END>");
}



TEST_F(HarpoonBattleTest, EnumToStringUndefined) {
   Death::SetupExitHandler();
   Death::ClearExits();
   EXPECT_FALSE(Death::WasKilled());
   auto wrong = HarpoonBattle::EnumToString(static_cast<HarpoonBattle::ReceivedType>(1000));
   auto found = wrong.find("<ERROR>");
   EXPECT_TRUE(found != std::string::npos) << wrong;
   EXPECT_TRUE(Death::WasKilled());

   auto foundDeath = Death::Message().find("Unknown ReceivedType:");
   EXPECT_TRUE(foundDeath != std::string::npos) << Death::Message();
}

TEST_F(HarpoonBattleTest, StringToEnum) {
   auto data = HarpoonBattle::StringToEnum("<DATA>");
   auto done = HarpoonBattle::StringToEnum("<DONE>");
   auto error = HarpoonBattle::StringToEnum("<ERROR>");
   auto end = HarpoonBattle::StringToEnum("<END>");
   auto wrong = HarpoonBattle::StringToEnum("<Hello World>");

   using namespace HarpoonBattle;
   EXPECT_TRUE(data == ReceivedType::Data);
   EXPECT_TRUE(done == ReceivedType::Done);
   EXPECT_TRUE(error == ReceivedType::Error);
   EXPECT_TRUE(end == ReceivedType::End);
   EXPECT_TRUE(wrong == ReceivedType::Error);
}


TEST_F(HarpoonBattleTest, MergeDataForDifferentTypes) {
   const std::string uuid = "some-uuid";
   auto type = KrakenBattle::SendType::Begin;
   auto error = std::string("no error - ignored");
   auto data = KrakenIntegrationHelper::GetRandomData(1024);
   auto merged = KrakenBattle::MergeData(uuid, type, data, error);

   EXPECT_EQ(merged.size(), 1040);  // 1024 + uuid + type
   merged.clear();
   EXPECT_EQ(merged.size(), 0);

   type = KrakenBattle::SendType::Data;
   merged = KrakenBattle::MergeData(uuid, type, data, error);  // 1024 + uuid + type
   EXPECT_EQ(merged.size(), 1039);
   merged.clear();
   EXPECT_EQ(merged.size(), 0);

   type = KrakenBattle::SendType::Error;
   merged = KrakenBattle::MergeData(uuid, type, data, error); // uuid + type + error msg
   EXPECT_EQ(merged.size(), 34);
   merged.clear();
   EXPECT_EQ(merged.size(), 0);

   type = KrakenBattle::SendType::End;
   merged = KrakenBattle::MergeData(uuid, type, data, error); // uuid + type + error msg.
   EXPECT_EQ(merged.size(), 41);
}

TEST_F(HarpoonBattleTest, ChunksExtractedDATA) {
   const std::string uuid = "some-uuid";
   auto type = KrakenBattle::SendType::Data;
   auto data = KrakenIntegrationHelper::GetRandomData(1024);
   auto error = std::string("no error - ignored");
   auto merged = KrakenBattle::MergeData(uuid, type, data, error);

   auto extracted = HarpoonBattle::ExtractToParts(merged);
   EXPECT_EQ(std::get<HarpoonBattle::IndexOfSession>(extracted), uuid);

   auto extractedType = std::get<HarpoonBattle::IndexOfReceivedType>(extracted);
   EXPECT_EQ(extractedType, HarpoonBattle::ReceivedType::Data);

   auto extractedChunk = std::get<HarpoonBattle::IndexOfChunk>(extracted);
   auto dataAsString = KrakenIntegrationHelper::vectorToString(data);
   auto extractedChunkAsString = KrakenIntegrationHelper::vectorToString(extractedChunk);
   EXPECT_TRUE(dataAsString == extractedChunkAsString) << "data: " << dataAsString.size() << ",  extracted: " << extractedChunkAsString.size();
   EXPECT_EQ(extractedChunkAsString.size(), 1024);
   EXPECT_EQ(dataAsString.size(), 1024);
}


TEST_F(HarpoonBattleTest, ChunksExtractedDONE) {
   const std::string uuid = "some-uuid";
   auto type = KrakenBattle::SendType::Done;
   auto ignored = KrakenIntegrationHelper::GetRandomData(1024);
   auto error = std::string("no error - ignored");
   auto merged = KrakenBattle::MergeData(uuid, type, ignored, error);

   auto extracted = HarpoonBattle::ExtractToParts(merged);
   EXPECT_EQ(std::get<HarpoonBattle::IndexOfSession>(extracted), uuid);

   auto extractedType = std::get<HarpoonBattle::IndexOfReceivedType>(extracted);
   EXPECT_EQ(extractedType, HarpoonBattle::ReceivedType::Done);

   auto extractedChunk = std::get<HarpoonBattle::IndexOfChunk>(extracted);
   auto dataAsString = KrakenIntegrationHelper::vectorToString(ignored);
   auto extractedChunkAsString = KrakenIntegrationHelper::vectorToString(extractedChunk);
   EXPECT_TRUE(dataAsString != extractedChunkAsString) << "data: " << dataAsString.size() << ",  extracted: " << extractedChunkAsString.size();
   EXPECT_EQ(extractedChunkAsString.size(), 0);
}


TEST_F(HarpoonBattleTest, ChunksExtractedEND) {
   const std::string uuid = "some-uuid";
   const std::string expectedUuid = "00000000-0000-0000-0000-000000000000";
   auto type = KrakenBattle::SendType::End;
   Kraken::Chunks ignored;
   auto error = std::string("no error - ignored");
   auto merged = KrakenBattle::MergeData(uuid, type, ignored, error);

   auto extracted = HarpoonBattle::ExtractToParts(merged);
   std::string extractedUuid = std::get<HarpoonBattle::IndexOfSession>(extracted);
   EXPECT_EQ(expectedUuid, extractedUuid);

   auto extractedType = std::get<HarpoonBattle::IndexOfReceivedType>(extracted);
   EXPECT_EQ(extractedType, HarpoonBattle::ReceivedType::End);

   auto extractedChunk = std::get<HarpoonBattle::IndexOfChunk>(extracted);
   auto dataAsString = KrakenIntegrationHelper::vectorToString(ignored);
   auto extractedChunkAsString = KrakenIntegrationHelper::vectorToString(extractedChunk);
   EXPECT_TRUE(dataAsString == extractedChunkAsString) << "data: " << dataAsString.size() << ",  extracted: " << extractedChunkAsString.size();
   EXPECT_EQ(extractedChunkAsString.size(), 0);
}



TEST_F(HarpoonBattleTest, ChunksExtractedERROR) {
   const std::string uuid = "some-uuid";
   auto type = KrakenBattle::SendType::Error;
   Kraken::Chunks ignored;
   auto error = std::string("has error - NOT ignored");
   auto merged = KrakenBattle::MergeData(uuid, type, ignored, error);

   auto extracted = HarpoonBattle::ExtractToParts(merged);
   EXPECT_EQ(std::get<HarpoonBattle::IndexOfSession>(extracted), uuid);

   
   auto extractedType = std::get<HarpoonBattle::IndexOfReceivedType>(extracted);
   EXPECT_EQ(extractedType, HarpoonBattle::ReceivedType::Error);

   auto extractedChunk = std::get<HarpoonBattle::IndexOfChunk>(extracted);
   auto dataAsString = KrakenIntegrationHelper::vectorToString(ignored);
   auto extractedChunkAsString = KrakenIntegrationHelper::vectorToString(extractedChunk);
   EXPECT_TRUE(dataAsString != extractedChunkAsString) << "data: " << dataAsString.size() << ",  extracted: " << extractedChunkAsString.size();

   EXPECT_TRUE(error == extractedChunkAsString) << "error: " << error.size() << ",  extracted: " << extractedChunkAsString.size();
   
   EXPECT_NE(extractedChunkAsString.size(), 0);
}


