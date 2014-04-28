/* 
 * File:   SystemSanityTest.h
 * Author: kjell
 *
 * Created on April 24, 2014, 8:15 AM
 */

#pragma once
#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

class SystemSanityTest : public ::testing::Test {
public:
   SystemSanityTest() : mTestDirectory("/tmp/TempSystemSanityTest"){}
   SystemSanityTest(const SystemSanityTest& orig);
   virtual ~SystemSanityTest() = default;

   virtual std::string CreateSubDirectory(const std::string& directory) {
      std::string mkTestSubDir{"mkdir -p " + mTestDirectory + "/" + directory};
      if (0 != system(mkTestSubDir.c_str())) {
         return {};
      }
      return {mTestDirectory + "/" + directory};
   }

   virtual std::string CreateFile(const std::string& directory, const std::string& filename) {
      std::string mkTestSubDir{"touch " + directory + "/" + filename};
      if (0 != system(mkTestSubDir.c_str())) {
         return {};
      }

      return { directory + "/" + filename };
   }

protected:

   virtual void SetUp() {
      std::string mkTestdir{"mkdir -p " + mTestDirectory};
      EXPECT_EQ(0, system(mkTestdir.c_str()));
   }

   virtual void TearDown() {
      std::string removeDir{"rm -rf " + mTestDirectory};
      EXPECT_EQ(0, system(removeDir.c_str()));
   }
   const std::string mTestDirectory;

};

