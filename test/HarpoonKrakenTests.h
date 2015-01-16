#pragma once

#include <gtest/gtest.h>
#include <FileIO.h>

class HarpoonKrakenTests : public ::testing::Test {
public:

   HarpoonKrakenTests() : mIpc("ipc:///tmp/HarpoonKrakenTests.ipc"){
      srand(std::time(nullptr));
   };

   int mPid;
   std::string mTcpLocation;
   std::string mInprocLocation;
   std::string mIpc;   

   static void* SendThreadNextChunkIdDie(void* arg);
   static void* SendThreadSendOneDie(void* arg);
   static void* SendThreadSendThirtyDie(void* arg);
   static void* SendThreadSendThirtyTwoEnd(void* arg);
   static void* SendHello(void* arg);
   static int GetTcpPort();
   static std::string GetTcpLocation(int port);

protected:

   virtual void SetUp() {
      zctx_interrupted = false;
   };

   virtual void TearDown() {
      FileIO::RemoveFileAsRoot(mIpc);
   };



};