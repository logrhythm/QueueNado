#pragma once

#include "gtest/gtest.h"
#include <vector>

#include "Death.h"

class DeathTest : public ::testing::Test {
public:

   DeathTest() {

   };
   
   static void EchoTheString(const Death::DeathCallbackArg& theString);
   static void RaceTest(const Death::DeathCallbackArg& theString);
   static bool ranEcho;
   static int ranTimes;
   static std::vector<Death::DeathCallbackArg> stringsEchoed;
protected:

   virtual void SetUp() {
      srand(std::time(NULL));
      ranEcho = false;
      ranTimes = 0;
      stringsEchoed.clear();
   };

   virtual void TearDown() {
      Death::Instance().ClearExits();
      
   };

private:
   
};



