#pragma once

#include <gtest/gtest.h>
#include "Shotgun.h"
#include "Alien.h"
#include <boost/lexical_cast.hpp>
#include <czmq.h>


class ShotgunAlienTests : public ::testing::Test {
 public:

   ShotgunAlienTests() {
      srand(std::time(NULL));
   };

   int mPid;
   std::string mIpcLocation;
   std::string mTcpLocation;
   std::string mInprocLocation;
   static void* ShotgunThread(void* args);
   static void* Buckshothread(void* args);
   static std::string GetTcpLocation();
   static std::string GetIpcLocation();
   static std::string GetInprocLocation();

   class ShotgunAmmo {
    public:

      ShotgunAmmo() :
         count(1), location(), delay(1) {
      }
      int count;
      std::string location;
      int delay;
   };

 protected:

   virtual void SetUp() {
      zctx_interrupted = false;
   }

   virtual void TearDown() {
      zctx_interrupted = false;
   }
 private:

};