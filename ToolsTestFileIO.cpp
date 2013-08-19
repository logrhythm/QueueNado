/* 
 * File:   ToolsTestFileIO.cpp
 * Author: kjell
 *
 * Created on August 15, 2013, 3:41 PM
 */

#include "gtest/gtest.h"
#include "FileIO.h"
#include <random>
#include <cstdio>


namespace {
   // Random integer function from http://www2.research.att.com/~bs/C++0xFAQ.html#std-random
   int random_int(int low, int high) {
      using namespace std;
      static std::random_device rd; // Seed with a real random value, if available
      static default_random_engine engine{rd()};
      typedef uniform_int_distribution<int> Distribution;
      static Distribution distribution{};
      return distribution(engine, Distribution::param_type{low, high});
   }

   struct ScopedFileCleanup
   {
      const std::string file;
      explicit ScopedFileCleanup(const std::string& name) : file(name){}
      ~ScopedFileCleanup() {remove(file.c_str()); }
   };
}



TEST(TestFileIO, CannotOpenFileToRead) {
   auto fileRead = FileIO::ReadAsciiFileContent({"/xyz/*&%/x.y.z"});
   EXPECT_TRUE(fileRead.result.empty());   
   EXPECT_FALSE(fileRead.error.empty());
   EXPECT_TRUE(fileRead.HasFailed());
}

TEST(TestFileIO, CanOpenFileToRead) {
   auto fileRead = FileIO::ReadAsciiFileContent({"/proc/stat"}); // fine as long as we are on *nix systems
   EXPECT_FALSE(fileRead.result.empty());   
   EXPECT_TRUE(fileRead.error.empty());
   EXPECT_FALSE(fileRead.HasFailed());
}

TEST(TestFileIO, CannotWriteToFile) {
   auto fileWrite = FileIO::WriteAsciiFileContent({"xyz/123/proc/stat"}, {"Hello World"}); 
   EXPECT_FALSE(fileWrite.result);   
   EXPECT_FALSE(fileWrite.error.empty());
   EXPECT_TRUE(fileWrite.HasFailed());
}

TEST(TestFileIO, CanWriteToFileAndReadTheFile) {
   using namespace std;
   
   string filename {"/tmp/TestFileIO_"};
   filename.append(to_string(random_int(0, 1000000)))
           .append({"_"})
           .append(to_string(random_int(0, 1000000)));

   // cleanup/removing the created file when exiting
   ScopedFileCleanup cleanup{filename};
   
   auto fileWrite = FileIO::WriteAsciiFileContent(filename, {"Hello World"}); 
   EXPECT_TRUE(fileWrite.result);   
   EXPECT_TRUE(fileWrite.error.empty());
   EXPECT_FALSE(fileWrite.HasFailed());
   
   auto fileRead = FileIO::ReadAsciiFileContent(filename);
   EXPECT_EQ("Hello World", fileRead.result);
   EXPECT_TRUE(fileRead.error.empty());
   EXPECT_FALSE(fileRead.HasFailed());
}



