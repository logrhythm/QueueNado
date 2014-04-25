#include "ToolsTestFileSystemWalker.h"
#include "FileSystemWalker.h"
#include "Result.h"
#include <g2log.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <thread>
#include <future>
#include <chrono>


namespace {
   struct FtsHelper {
      size_t fileCounter;
      size_t directoryCounter;
      size_t ignoredCounter;
      std::vector<std::string> names;
      std::vector<std::string> rootNames;

      FtsHelper() : fileCounter(0), directoryCounter(0), ignoredCounter(0) { }

      int FtsHelperAction(FTSENT* ptr, int flag) {
         bool ignored = false;
         switch (flag) {
            case FTS_DP: ++directoryCounter;
               break;
            case FTS_F: ++fileCounter;
               break;
            default:
               ignored = true;
               ++ignoredCounter;

               LOG(DEBUG) << "ignoring: " << std::string {
                  ptr->fts_name
               };
         }

         if (!ignored) {

            LOG(DEBUG) << "found: " << std::string {
               ptr->fts_name
            };
            names.push_back(std::string{ptr->fts_name});
            rootNames.push_back(std::string{ptr->fts_path});
         }
         return 0;
      }
   };

   /** Random int function from http://www2.research.att.com/~bs/C++0xFAQ.html#std-random */
   int random_int(int low, int high) {
      using namespace std;
      static std::random_device rd; // Seed with a real random value, if available
      static default_random_engine engine{rd()};
      typedef uniform_int_distribution<int> Distribution;
      static Distribution distribution{};

      return distribution(engine, Distribution::param_type{low, high});
   }
}


TEST_F(ToolsTestFileSystemWalker, Invalid){
   std::string invalidPath = "/xyx/Does/Not/Exist";   

   FtsHelper helper;
   auto FtsRealHelper = [&](FTSENT* ptr, int flag){
      return helper.FtsHelperAction(ptr, flag);
   };
   
   
   FileSystemWalker walker(invalidPath, FtsRealHelper);
   EXPECT_FALSE(walker.IsValid());
   EXPECT_TRUE(walker.Action().HasFailed());
}


TEST_F(ToolsTestFileSystemWalker, Valid){   

   FtsHelper helper;
   auto FtsRealHelper = [&](FTSENT* ptr, int flag){
      return helper.FtsHelperAction(ptr, flag);
   };
      
   FileSystemWalker walker(mTestDirectory, FtsRealHelper);
   EXPECT_FALSE(walker.Action().HasFailed());
   
   EXPECT_EQ(helper.fileCounter, 0);
   EXPECT_EQ(helper.ignoredCounter, 1); // directory we're in PRE-traversal
   EXPECT_EQ(helper.directoryCounter, 1); // directory we're in POST-traversal
   
   ASSERT_EQ(helper.names.size(), 1);
   ASSERT_EQ(helper.names[0], "TempFileSystemWalker");
   
   ASSERT_EQ(helper.rootNames.size(), 1);
   ASSERT_EQ(helper.rootNames[0], "/tmp/TempFileSystemWalker");
}

void ToolsTestFileSystemWalker::ThreeDirectoriesThreeFiles() {
    std::this_thread::sleep_for(std::chrono::milliseconds(random_int(0, 100)));

   FtsHelper helper;
   auto FtsRealHelper = [&](FTSENT* ptr, int flag){
      return helper.FtsHelperAction(ptr, flag);
   };
   
   
    std::string dir1 = "/tmp/TempFileSystemWalker/dir1";
    std::string dir2 = "/tmp/TempFileSystemWalker/dir1/dir2";
    std::string dir3 = "/tmp/TempFileSystemWalker/dir3";
    std::string file1 = {dir1 + "/file_1"};
    std::string file2 = {dir1 + "/file_2"};
    std::string file3 = {dir3 + "/file_3"};
    
   EXPECT_TRUE(FileIO::DoesDirectoryExist("/tmp/TempFileSystemWalker/dir1/"));
   EXPECT_TRUE(FileIO::DoesDirectoryExist("/tmp/TempFileSystemWalker/dir1/dir2"));
   EXPECT_TRUE(FileIO::DoesDirectoryExist(dir2));
   EXPECT_EQ(dir2, "/tmp/TempFileSystemWalker/dir1/dir2");
   
   EXPECT_TRUE(FileIO::DoesDirectoryExist("/tmp/TempFileSystemWalker/dir3/"));
   EXPECT_TRUE(FileIO::DoesFileExist(file1)); ///tmp/TempFileSystemWalker/dir1/file_0
   EXPECT_TRUE(FileIO::DoesFileExist(file2)); //tmp/TempFileSystemWalker/dir2/file_11
   EXPECT_TRUE(FileIO::DoesFileExist(file3)); //tmp/TempFileSystemWalker/dir2/file_11
   
   
   FileSystemWalker walker("/tmp/TempFileSystemWalker", FtsRealHelper);
   
   EXPECT_TRUE(walker.IsValid());
   Result<int> result = walker.Action();
   EXPECT_FALSE(result.HasFailed());
   
   
   ASSERT_EQ(helper.rootNames.size(), helper.names.size());
   EXPECT_EQ(helper.names.size(), 7); // 3files, 3 directories, + starting directory
   for(size_t index = 0; index < helper.names.size(); ++index) {
     LOG(DEBUG) << "[path][entity_name]: [" << helper.rootNames[index] << "][" << helper.names[index] <<"]";
   }

   
   EXPECT_EQ(helper.fileCounter, 3);
   EXPECT_EQ(helper.ignoredCounter, 4); // directories: 3 + where we are PRE-traversal
   EXPECT_EQ(helper.directoryCounter, 4); // directories: 3 + where we are POST-traversal
   
   EXPECT_TRUE(helper.rootNames.end() != std::find(helper.rootNames.begin(), helper.rootNames.end(), file1));
   EXPECT_TRUE(helper.rootNames.end() != std::find(helper.rootNames.begin(), helper.rootNames.end(), file2));
   EXPECT_TRUE(helper.rootNames.end() != std::find(helper.rootNames.begin(), helper.rootNames.end(), file3));
   EXPECT_TRUE(helper.rootNames.end() != std::find(helper.rootNames.begin(), helper.rootNames.end(), dir1)) << dir1;
   EXPECT_TRUE(helper.rootNames.end() != std::find(helper.rootNames.begin(), helper.rootNames.end(), dir2));
   EXPECT_TRUE(helper.rootNames.end() != std::find(helper.rootNames.begin(), helper.rootNames.end(), dir3));
   
   EXPECT_TRUE(helper.names.end() != std::find(helper.names.begin(), helper.names.end(), "dir1"));
   EXPECT_TRUE(helper.names.end() != std::find(helper.names.begin(), helper.names.end(), "dir2"));
   EXPECT_TRUE(helper.names.end() != std::find(helper.names.begin(), helper.names.end(), "dir3"));
   EXPECT_TRUE(helper.names.end() != std::find(helper.names.begin(), helper.names.end(), "file_1"));
   EXPECT_TRUE(helper.names.end() != std::find(helper.names.begin(), helper.names.end(), "file_2"));
   EXPECT_TRUE(helper.names.end() != std::find(helper.names.begin(), helper.names.end(), "file_3"));
}


TEST_F(ToolsTestFileSystemWalker, ThreeDirectoriesThreeFiles_SameThread){   
   auto dir1 = CreateSubDirectory("dir1");
   auto dir2 = CreateSubDirectory("dir1/dir2");
   auto dir3 = CreateSubDirectory("dir3");
   
   CreateFile({mTestDirectory + "/dir1"}, {"file_1"});
   CreateFile({mTestDirectory + "/dir1"}, {"file_2"});
   CreateFile({mTestDirectory + "/dir3"}, {"file_3"});
   
   ThreeDirectoriesThreeFiles();
}


TEST_F(ToolsTestFileSystemWalker, ThreeDirectoriesThreeFiles_TwoThreads){   
   auto dir1 = CreateSubDirectory("dir1");
   auto dir2 = CreateSubDirectory("dir1/dir2");
   auto dir3 = CreateSubDirectory("dir3");
   
   CreateFile({mTestDirectory + "/dir1"}, {"file_1"});
   CreateFile({mTestDirectory + "/dir1"}, {"file_2"});
   CreateFile({mTestDirectory + "/dir3"}, {"file_3"});

   auto future0 = std::async(std::launch::async, &ToolsTestFileSystemWalker::ThreeDirectoriesThreeFiles, this);
   ThreeDirectoriesThreeFiles();
   future0.get();
}


TEST_F(ToolsTestFileSystemWalker, ThreeDirectoriesThreeFiles_100Threads) {
   
   auto dir1 = CreateSubDirectory("dir1");
   auto dir2 = CreateSubDirectory("dir1/dir2");
   auto dir3 = CreateSubDirectory("dir3");
   
   auto file1 = CreateFile({mTestDirectory + "/dir1"}, {"file_1"});
   auto file2 = CreateFile({mTestDirectory + "/dir1"}, {"file_2"});
   auto file3 = CreateFile({mTestDirectory + "/dir3"}, {"file_3"});
   
   EXPECT_TRUE(FileIO::DoesDirectoryExist(dir1));
   EXPECT_TRUE(FileIO::DoesDirectoryExist(dir2));
   EXPECT_TRUE(FileIO::DoesDirectoryExist(dir3));
   EXPECT_EQ(dir2, "/tmp/TempFileSystemWalker/dir1/dir2");
   
   EXPECT_TRUE(FileIO::DoesFileExist(file1)); 
   EXPECT_TRUE(FileIO::DoesFileExist(file2)); 
   EXPECT_TRUE(FileIO::DoesFileExist(file3)); 
   
   std::vector<std::future<void>> futures;
   for (size_t index = 0; index < 100; ++index) {
      futures.push_back(std::async(std::launch::async, &ToolsTestFileSystemWalker::ThreeDirectoriesThreeFiles, this));
   }
   
   for(auto& result : futures) {
      result.get();
   }
}