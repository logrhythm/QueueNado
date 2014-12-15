#pragma once

#include "gtest/gtest.h"
#include "boost/lexical_cast.hpp"

class FileRecvTests : public ::testing::Test {
public:

   FileRecvTests() {
      srand(std::time(NULL));
   };

   int mPid;
   std::string mIpcLocation;
   std::string mTcpLocation;
   std::string mInprocLocation;
   static void * SendThreadNextChunkIdDie(void * arg);
   static void * SendThreadSendOneDie(void *arg);
   static void * SendThreadSendThirtyDie(void * arg);
   static void * SendThreadSendThirtyTwoEnd(void * arg);
   static int GetTcpPort();
   static std::string GetTcpLocation(int port);
   static std::string GetIpcLocation();
   static std::string GetInprocLocation();

protected:

   virtual void SetUp() {
      zctx_interrupted = false;
   };

   virtual void TearDown() {
   };
private:

};