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
#include "ProcessManager.h"
#include "MockConfMaster.h"

class RaIIFolderUsage : public ::testing::Test {
public:

   RaIIFolderUsage() {
   };
protected:

   virtual void SetUp() {
      ProcessManager::Instance();
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

   virtual std::string CreateSubDirectory(const std::string& directory) {
      std::string mkTestSubDir{"mkdir -p " + testDir.str() + "/" + directory};
      if (0 != system(mkTestSubDir.c_str())) {
         return {};
      }
      return { testDir.str() + "/" + directory };
   }

   virtual std::string CreateFile(const std::string& directory, const std::string& filename) {
      std::string mkFile{"touch " + directory + "/" + filename};
      if (0 != system(mkFile.c_str())) {
         return  {};
      }
      return {directory + "/" + filename };
   }

   std::stringstream testDir;
   MockConfMaster mConf;
private:

};






