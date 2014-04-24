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
   static_assert(8 == sizeof (size_t), "Error: 64-bit support failed. For improved architecture this check can be sizeof(size_t) >= 8");
   EXPECT_TRUE(true);
}


//
// The purpose of this test is to verify if various GLIBC functions are thread safe when we 
// are using full path, and not relative path. 
//
// For relative path we likely have an issue but we should never use relative path!
// Bug details:
// ============
// Internally the GLIBC functions (chmod, remove, chown etc) will use fts_open. 
// If fts_open is not called with a specific flag "FTS_NOCHDIR" then it is thread
// unsafe as another thread using fts_open can cause a change of the active directory. 
//
//
TEST_F(SystemSanityTest, GlibCallsUsingFullPath__ShouldNotAffectOtherDirectories) {

   std::string dirRemove = CreateSubDirectory("to_remove");
   std::string dirChange1 = CreateSubDirectory("to_change1");
   std::string dirChange2 = CreateSubDirectory("to_change2");

   const size_t maxFiles = 1000;
   for (size_t index = 0; index < maxFiles; ++index) {
      CreateFile(dirRemove, std::to_string(index));
      CreateFile(dirChange1, std::to_string(index));
      CreateFile(dirChange2, std::to_string(index));
   }


   std::vector < std::future<bool>> finished;
   
   // lambda function for changing permissions on files
   auto changeChmod = [&maxFiles](const std::string& path, mode_t mode) ->bool {
      bool success = true;
      for (size_t index = 0; index < maxFiles; ++index) {
         std::string filepath = {path + "/" + std::to_string(index)};
         success = (0 == chmod(filepath.c_str(), mode)) && success;
      }

      return success;
   };

   // lambda function for removing files
   auto removeFiles = [&maxFiles](const std::string& path) ->bool {
      bool success = true;
      for (size_t index = 0; index < maxFiles; ++index) {
         std::string filepath = {path + "/" + std::to_string(index)};
         success = (0 == remove(filepath.c_str())) && success;
      }
      return success;
   };


   finished.push_back(std::async(std::launch::async, changeChmod, dirChange1, 777));
   finished.push_back(std::async(std::launch::async, changeChmod, dirChange2, 555));
   finished.push_back(std::async(std::launch::async, removeFiles, dirRemove));

   for (auto& done : finished) {
      EXPECT_TRUE(done.get());
   }

   bool success = true;
   for (size_t index = 0; success && (index < maxFiles); ++index) {
      EXPECT_FALSE(FileIO::DoesFileExist({dirRemove + "/" + std::to_string(index)}));


      std::string changed1 = {dirChange1 + "/" + std::to_string(index)};
      std::string changed2 = {dirChange2 + "/" + std::to_string(index)};
      EXPECT_TRUE(FileIO::DoesFileExist(changed1));
      EXPECT_TRUE(FileIO::DoesFileExist(changed2));

      struct stat buf1;
      EXPECT_EQ(0,  stat(changed1.c_str(), &buf1));
      EXPECT_EQ(777, buf1.st_mode & 777);
      EXPECT_NE(555, buf1.st_mode & 555);

      struct stat buf2;
      EXPECT_EQ(0, stat(changed2.c_str(), &buf2));
      EXPECT_EQ(555, buf2.st_mode & 555);
      EXPECT_NE(777, buf2.st_mode & 777);
   }

}


