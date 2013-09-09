#pragma once

#include "gtest/gtest.h"
#include "ElasticSearch.h"
#include "czmq.h"

class ElasticSearchTest : public ::testing::Test {
public:

   ElasticSearchTest() :targetIterations(1000) {
      std::stringstream sS;

      sS << "ipc:///tmp/elasticSearchtest" << pthread_self();
      mAddress = sS.str();
   };

protected:

   virtual void SetUp() {
      zctx_interrupted = false;
   }

   virtual void TearDown() {
   }
   std::string mAddress;
   const int targetIterations;
};
