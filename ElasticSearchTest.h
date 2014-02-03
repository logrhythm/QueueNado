#pragma once

#include "gtest/gtest.h"
#include "ElasticSearch.h"
#include "czmq.h"

class ElasticSearchTest : public ::testing::Test {
public:

   ElasticSearchTest() : targetIterations(100), totalHits(0) {
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
      totalHits = 0;
      
   }

   virtual void TearDown() {
   }
   std::string mAddress;
   std::string mBigRecord;
   const int targetIterations;
   size_t totalHits;
};
