/*
 * File:   KrakenBattleTest.h
 * Author: Kjell Hedstrom
 *
 * Created on November 24, 2015
 */

#pragma once

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

class KrakenBattleTest : public ::testing::Test {
public:
   KrakenBattleTest() :
      mTestDirectory("/tmp/TestOfKrakenBattleTest"),
      mCreateTestDirectory("mkdir -p " + mTestDirectory),
      mTearDownTestDirectory("rm -rf " + mTestDirectory) {}

   virtual ~KrakenBattleTest() = default;

protected:
   virtual void SetUp() {
      system(mTearDownTestDirectory.c_str());
      EXPECT_EQ(0, system(mCreateTestDirectory.c_str()));
   }

   virtual void TearDown() {
      EXPECT_EQ(0, system(mTearDownTestDirectory.c_str()));
   }

   std::string mTestDirectory;
   std::string mCreateTestDirectory;
   std::string mTearDownTestDirectory;
};