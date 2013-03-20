#pragma once

#include "SendStats.h"

class MockSendStats : public SendStats {
public:

   MockSendStats() : SendStats(), mDummyTimeStamp(0) {
   }

   std::time_t GetTimestamp() {
      return mDummyTimeStamp;
   }

   std::time_t mDummyTimeStamp;

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, double statValue) {
      return SendStats::ConstructMessageToSend(key, statValue, false);
   }

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, float statValue) {
      return SendStats::ConstructMessageToSend(key, statValue, false);
   }

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, int statValue) {
      return SendStats::ConstructMessageToSend(key, statValue, false);
   }

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, const std::string& statName, int statValue) {
      return SendStats::ConstructMessageToSend(key, statName, statValue, false);
   }

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, const std::string& statName, int statValue, const std::string& columnName) {
      return SendStats::ConstructMessageToSend(key, statName, statValue, columnName, false);
   }

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, unsigned int statValue) {
      return SendStats::ConstructMessageToSend(key, statValue, false);
   }

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, uint64_t statValue) {
      return SendStats::ConstructMessageToSend(key, statValue, false);
   }

   protoMsg::Stats& ConstructMessageToSend(const std::string& key, int64_t statValue) {
      return SendStats::ConstructMessageToSend(key, statValue, false);
   }

};

