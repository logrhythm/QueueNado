/* 
 * File:   ToolsTest.h
 * Author: kjell
 *
 * Created on August 1, 2013, 4:57 PM
 */

#pragma once
#include "gtest/gtest.h"

struct Simple {
   bool& alive;
   Simple(bool& flag) : alive(flag) {  alive = true;  }
   ~Simple() {  alive = false; }
};

class ToolsTest : public ::testing::Test {
public:
   ToolsTest() {   };
protected:
   virtual void SetUp() {   };
   virtual void TearDown() {   };
private:
};