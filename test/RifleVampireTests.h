#pragma once


#include "gtest/gtest.h"
#include "Rifle.h"
#include "Vampire.h"

#include <atomic>
#include <sys/time.h>
#include <czmq.h>

class RifleVampireTests : public ::testing::Test {
public:

   RifleVampireTests() {
   };

protected:

   virtual void SetUp() {
   };

   virtual void TearDown() {   };

private:

};
