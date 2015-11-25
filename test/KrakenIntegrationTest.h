/*
 * File:   KrakenIntegrationTest.h
 * Author: Kjell Hedstrom
 *
 * Created on November 24, 2015
 */

#pragma once

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>
#include <czmq.h>

class KrakenIntegrationTest : public ::testing::Test {
public:
   KrakenIntegrationTest() :
      mTestDirectory("/tmp/TestOfKrakenIntegrationTest"),
      mCreateTestDirectory("mkdir -p " + mTestDirectory),
      mTearDownTestDirectory("rm -rf " + mTestDirectory) {}

   virtual ~KrakenIntegrationTest() = default;

protected:
   virtual void SetUp() {
      zctx_interrupted = false;
      system(mTearDownTestDirectory.c_str());
      EXPECT_EQ(0, system(mCreateTestDirectory.c_str()));
   }

   virtual void TearDown() {
      zctx_interrupted = false;
      EXPECT_EQ(0, system(mTearDownTestDirectory.c_str()));
   }

   std::string mTestDirectory;
   std::string mCreateTestDirectory;
   std::string mTearDownTestDirectory;
};