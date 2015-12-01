/*
 * File:   HarpoonBattleTest.h
 * Author: Kjell Hedstrom
 *
 * Created on November 30, 2015
 */

#pragma once

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

class HarpoonBattleTest : public ::testing::Test {
public:
   HarpoonBattleTest() :
      mTestDirectory("/tmp/TestOfHarpoonBattleTest"),
      mCreateTestDirectory("mkdir -p " + mTestDirectory),
      mTearDownTestDirectory("rm -rf " + mTestDirectory) {}

   virtual ~HarpoonBattleTest() = default;

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