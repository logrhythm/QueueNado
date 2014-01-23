/* 
 * File:   DiskUsageTest.h
 * Author: kjell
 *
 * Created on September 4, 2013, 10:34 AM
 */

#pragma once
#include <gtest/gtest.h>
#include <sstream>
#include <cstdio>
#include <cstdlib>

class RaIIFolderUsage : public ::testing::Test {
public:

   RaIIFolderUsage() {
   };
protected:

   virtual void SetUp() {
      
      testDir << "/tmp/TestSize";

      std::string makeADir;
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      system(makeADir.c_str());

      makeADir = "mkdir -p ";
      makeADir += testDir.str();
      system(makeADir.c_str());
   };

   virtual void TearDown() {
      std::string makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
   };

   std::stringstream testDir;
private:

};






