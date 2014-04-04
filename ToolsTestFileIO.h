#pragma once
#include "gtest/gtest.h"
#include <cstdlib>
#include <string>
#include "FileIO.h"
#include <iostream>
class TestFileIO : public ::testing::Test {
public:

   TestFileIO() : mTestDirectory("/tmp/TempDirectoryTestOfFileIO") {}
   virtual ~TestFileIO() = default;

   virtual bool CreateSubDirectory(const std::string& directory) {
      std::string mkTestSubDir{"mkdir -p " + mTestDirectory + "/" + directory};
      return (0 == system(mkTestSubDir.c_str()));
   }

   virtual bool CreateFile(const std::string& directory, const std::string& filename) {
      std::string mkTestSubDir{"touch " + directory + "/" + filename};
      return (0 == system(mkTestSubDir.c_str()));
   }
   
//   FileIO::DirectoryReader::Found  FastForward(FileIO::DirectoryReader& reader){
//      auto fileAndType = reader.Next();
//      while ((fileAndType.second == ".") || (fileAndType.second == "..")) {
//         fileAndType = reader.Next();
//      }
//      return fileAndType;
//   }
   

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
