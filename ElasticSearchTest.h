#pragma once

#include "gtest/gtest.h"
#include "ElasticSearch.h"
#include "czmq.h"

class ElasticSearchTest : public ::testing::Test {
public:

   ElasticSearchTest() : targetIterations(1000) {
      std::stringstream sS;

      sS << "ipc:///tmp/elasticSearchtest" << pthread_self();
      mAddress = sS.str();
   };

protected:

   virtual void SetUp() {
      zctx_interrupted = false;
      std::fstream goodResult("resources/bigrecord", std::ios_base::in);
      goodResult >> mBigRecord;
      goodResult.close();
      
   }

   virtual void TearDown() {
   }
   std::string mAddress;
   std::string mBigRecord;
   const int targetIterations;
};
