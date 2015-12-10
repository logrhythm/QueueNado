/* 
 * File:   QueueWatchDogTest.h
 * Author: Craig Cogdill
 * Date Created: August 18, 2015 1:14PM
 */

#pragma once
#include "gtest/gtest.h"
#include <memory>
#include <csignal>

class QueueWatchDogTest : public ::testing::Test {
public:

   QueueWatchDogTest() {};
   virtual ~QueueWatchDogTest(){}

protected:
   virtual void SetUp() {
   };

   virtual void TearDown() {
      
   };
private:
};