#pragma once

#include <gtest/gtest.h>

class FileSendTests : public ::testing::Test {
public:

   FileSendTests() {
      srand(std::time(NULL));
   };

   int mPid;
   std::string mIpcLocation;
   std::string mTcpLocation;
   std::string mInprocLocation;
   static void* RecvThreadNextChunkIdDie(void* arg);
   static void* RecvThreadNextChunkIdWait(void* arg);
   static void* RecvThreadGetThreeWait(void* arg);
   static void* RecvThreadGetFileDie(void* arg);
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