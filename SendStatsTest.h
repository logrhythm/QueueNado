#pragma once

#include "gtest/gtest.h"
#include "Rifle.h"
#include "Vampire.h"
#include "SendStats.h"
#include "boost/lexical_cast.hpp"


class SendStatsTest : public ::testing::Test {
public:
    SendStatsTest() {
      mStatsIpcLocation = GetIpcLocation();
    }
    void VampireThread(int numberOfMessages,
           std::string& location, std::string& exampleData, int hwm, int ioThreads);
    static std::string GetIpcLocation();

    SendStats mSender;
    std::string mStatsIpcLocation;

};
