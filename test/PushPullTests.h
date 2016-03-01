#pragma once


#include "gtest/gtest.h"
#include "Push.h"
#include "Pull.h"

#include <atomic>
#include <sys/time.h>
#include <czmq.h>

class PushPullTests : public ::testing::Test {
public:

   PushPullTests() {
   };

protected:

   virtual void SetUp() {
   };

   virtual void TearDown() {   };

private:

};
