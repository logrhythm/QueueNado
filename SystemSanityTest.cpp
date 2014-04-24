/* 
 * File:   SystemSanityTest.cpp
 * Author: kjell
 * 
 * Created on April 24, 2014, 8:15 AM
 */

#include "SystemSanityTest.h"
#include "FileIO.h"
#include <vector>
#include <future>
#include <thread>
#include <sys/stat.h> // mkdir
#include <cstdio>     // remove
TEST_F(SystemSanityTest, CompilerCheck_Assert_System_Expectations) {
   static_assert(1 == alignof(char), "Error: This should never fail. What is going on? Alignment of char should ALWAYS be 1");
   static_assert(8 == sizeof(size_t), "Error: 64-bit support failed. For improved architecture this check can be sizeof(size_t) >= 8");
   EXPECT_TRUE(true);   
}



TEST_F(SystemSanityTest, GlibC_FullPath__ShouldNotAffectOtherDirectories) {
   
   std:;string dirRemove = CreateSubDirectory("to_remove");
   std::string dirChange = CreateSubDirectory("to_change");
      
   const size_t maxFiles = 10000;
   for (size_t index = 0; index < maxFiles; ++index) {
      CreateFile(dirRemove, std::to_string(index));
      CreateFile(dirChange, std::to_string(index));
   }
   
   
   std::vector<std::future<void>> finished;
   auto changeChmod = [](const std::string& path) {
      
      
   }
   
    
   
   

   
      auto dir1 = CreateSubDirectory(std::to_string(dirIndex));
      CreateFile(dir1, {"f_" + std::to_string(1)});
      CreateFile(dir1, {"f_" + std::to_string(2)});
   
      auto dir2 = CreateSubDirectory({"/to_remove/" + std::to_string(dirIndex)});
      CreateFile(dir2, {"f_" + std::to_string(1)});
      CreateFile(dir2, {"f_" + std::to_string(2)});
   }
   
   
   auto createDir = [](const std::string& fullPathName) -> bool {
      return (0 == mkdir(fullPathName.c_str(), 777));
   }

   // will call unlink for file and rmdir for directories
   auto deleteEntity = [](const std::string& fullPathName) -> bool {
      return (0 == remove(fullPathName.c_str()));
   }
   
   auto changePermissions [](const std::string& fullPathName) -> bool {
      return (0 == chmod(fullPathName.c_str(), 777));
   }
   
   // Sanity check that empty directories can be deleted
   EXPECT_TRUE(createDir({mTestDirectory + "/remove_me2"}));
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/remove_me2"));
   EXPECT_FALSE(createDir({mTestDirectory + "/remove_me2"}));
   
   
   std::vector<std::future<bool>> asyncDoneStatus;
   for(size_t index = 0; index < 16000; ++index) {
      asyncDoneStatus.push_back(std::async(std::launch::async()));
      
      
      
      
   }
   
//   
//   for (size_t dirIndex = 0; dirIndex < maxDirectories; ++dirIndex) {
//      auto dir1 = CreateSubDirectory(std::to_string(dirIndex));
//      auto dir2 = CreateSubDirectory({"/to_remove/" + std::to_string(dirIndex)});
//   
//      CreateFile(dir1, {"f_" + std::to_string(1)});
//      CreateFile(dir1, {"f_" + std::to_string(2)});
//   }
   
   
   
   // Verify setup
   //EXPECT_TRUE()
   

}