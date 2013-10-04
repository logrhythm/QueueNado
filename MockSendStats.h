#pragma once

#include "SendStats.h"
#include "include/global.h"
#include <vector>
#include <string>

class MockSendStats : public SendStats {
public:

   MockSendStats() : SendStats(), mDummyTimeStamp(0) {
   }

   std::time_t GetTimestamp() {
      return mDummyTimeStamp;
   }

   std::time_t mDummyTimeStamp;
   std::vector<int64_t> mSendStatValues;
   std::vector<std::string> mSendStatKeys;

   bool SendStat(const std::string& key, const int64_t statValue,
           const bool repeatStat) LR_OVERRIDE {
      mSendStatKeys.push_back(key);
      mSendStatValues.push_back(statValue);
      return SendStats::SendStat(key, statValue, repeatStat);

   }

   protoMsg::Stats& GetInternalMessage() {
      return SendStats::GetInternalMessage();
   }

   void ConstructMessageToSend(const std::string& key, double statValue) {
      SendStats::ConstructMessageToSend(key, statValue, false);
   }

   void ConstructMessageToSend(const std::string& key, float statValue) {
      SendStats::ConstructMessageToSend(key, statValue, false);
   }

   void ConstructMessageToSend(const std::string& key, int statValue) {
      SendStats::ConstructMessageToSend(key, statValue, false);
   }

   void ConstructMessageToSend(const std::string& key,
           const std::string& statName, const int64_t statValue,
           const std::string& columnName, const bool repeatStat) {
      SendStats::ConstructMessageToSend(key, statName, statValue, columnName, repeatStat);
   }

   void ConstructMessageToSend(const std::string& key, const int64_t statValue, const bool repeatStat) {
      SendStats::ConstructMessageToSend(key, statValue, repeatStat);
   }

   void ConstructMessageToSend(const std::string& key, const std::string& statName, int statValue) {
      SendStats::ConstructMessageToSend(key, statName, statValue, false);
   }

   void ConstructMessageToSend(const std::string& key, const std::string& statName, int statValue, const std::string& columnName) {
      SendStats::ConstructMessageToSend(key, statName, statValue, columnName, false);
   }

   void ConstructMessageToSend(const std::string& key, unsigned int statValue) {
      SendStats::ConstructMessageToSend(key, statValue, false);
   }

   void ConstructMessageToSend(const std::string& key, uint64_t statValue) {
      SendStats::ConstructMessageToSend(key, statValue, false);
   }

   void ConstructMessageToSend(const std::string& key, int64_t statValue) {
      SendStats::ConstructMessageToSend(key, statValue, false);
   }

};

