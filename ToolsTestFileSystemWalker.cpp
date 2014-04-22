#include "ToolsTestFileSystemWalker.h"
#include "FileSystemWalker.h"
#include "Result.h"
#include <g2log.hpp>
#include <vector>
#include <string>
#include <algorithm>
namespace {
   size_t gFileCounter{0};
   size_t gDirectoryCounter{0};
   size_t gIgnoredCounter{0};
   std::vector<std::string> gNames;
   std::vector<std::string> gRootNames;
   
   
   void clear() {
      gFileCounter=0;
      gDirectoryCounter = 0;
      gIgnoredCounter = 0;
      gNames.clear();
      gRootNames.clear();
   }
   
   
   int FtsHelper(FTSENT* ptr, int flag) {        
      
      bool ignored = false;
      switch(flag) {
       case FTS_DP : ++gDirectoryCounter; break;
       case FTS_F : ++gFileCounter; break;
       default:
         ignored = true;
         ++gIgnoredCounter;
         LOG(DEBUG) << "ignoring: " << std::string{ptr->fts_name};
      }
      
      if (!ignored) {
         LOG(DEBUG) << "found: " << std::string{ptr->fts_name};
         gNames.push_back(std::string{ptr->fts_name});
         gRootNames.push_back(std::string{ptr->fts_path});
      }

      return 0;
   }
}


TEST_F(ToolsTestFileSystemWalker, Invalid){
   clear();
   std::string invalidPath = "/xyx/Does/Not/Exist";   
   
   FileSystemWalker walker(invalidPath, FtsHelper);
   EXPECT_FALSE(walker.IsValid());
   EXPECT_TRUE(walker.Action().HasFailed());
}


TEST_F(ToolsTestFileSystemWalker, Valid){   
   clear();
   
   FileSystemWalker walker(mTestDirectory, FtsHelper);
   EXPECT_FALSE(walker.Action().HasFailed());
   
   EXPECT_EQ(gFileCounter, 0);
   EXPECT_EQ(gIgnoredCounter, 1); // directory we're in PRE-traversal
   EXPECT_EQ(gDirectoryCounter, 1); // directory we're in POST-traversal
   
   ASSERT_EQ(gNames.size(), 1);
   ASSERT_EQ(gNames[0], "TempFileSystemWalker");
   
   ASSERT_EQ(gRootNames.size(), 1);
   ASSERT_EQ(gRootNames[0], "/tmp/TempFileSystemWalker");
}


TEST_F(ToolsTestFileSystemWalker, ThreeDirectoriesThreeFiles){   
   clear();
   
   auto dir1 = CreateSubDirectory("dir1");
   auto dir2 = CreateSubDirectory("dir1/dir2");
   auto dir3 = CreateSubDirectory("dir3");
   
   auto file1 = CreateFile({mTestDirectory + "/dir1"}, {"file_1"});
   auto file2 = CreateFile({mTestDirectory + "/dir1"}, {"file_2"});
   auto file3 = CreateFile({mTestDirectory + "/dir3"}, {"file_3"});
   
   
   FileSystemWalker walker("/tmp/TempFileSystemWalker", FtsHelper);
   EXPECT_TRUE(FileIO::DoesDirectoryExist("/tmp/TempFileSystemWalker/dir1/"));
   EXPECT_TRUE(FileIO::DoesDirectoryExist("/tmp/TempFileSystemWalker/dir1/dir2"));
   EXPECT_TRUE(FileIO::DoesDirectoryExist(dir2));
   EXPECT_EQ(dir2, "/tmp/TempFileSystemWalker/dir1/dir2");
   
   EXPECT_TRUE(FileIO::DoesDirectoryExist("/tmp/TempFileSystemWalker/dir3/"));
   EXPECT_TRUE(FileIO::DoesFileExist(file1)); ///tmp/TempFileSystemWalker/dir1/file_0
   EXPECT_TRUE(FileIO::DoesFileExist(file2)); //tmp/TempFileSystemWalker/dir2/file_11

   
   EXPECT_TRUE(walker.IsValid());
   Result<int> result = walker.Action();
   EXPECT_FALSE(result.HasFailed());
   
   
   ASSERT_EQ(gRootNames.size(), gNames.size());
   EXPECT_EQ(gNames.size(), 7); // 3files, 3 directories, + starting directory
   for(size_t index = 0; index < gNames.size(); ++index) {
     LOG(DEBUG) << "[path][entity_name]: [" << gRootNames[index] << "][" << gNames[index] <<"]";
   }

   
   EXPECT_EQ(gFileCounter, 3);
   EXPECT_EQ(gIgnoredCounter, 4); // directories: 3 + where we are PRE-traversal
   EXPECT_EQ(gDirectoryCounter, 4); // directories: 3 + where we are POST-traversal
   
   EXPECT_TRUE(gRootNames.end() != std::find(gRootNames.begin(), gRootNames.end(), file1));
   EXPECT_TRUE(gRootNames.end() != std::find(gRootNames.begin(), gRootNames.end(), file2));
   EXPECT_TRUE(gRootNames.end() != std::find(gRootNames.begin(), gRootNames.end(), file3));
   EXPECT_TRUE(gRootNames.end() != std::find(gRootNames.begin(), gRootNames.end(), dir1));
   EXPECT_TRUE(gRootNames.end() != std::find(gRootNames.begin(), gRootNames.end(), dir2));
   EXPECT_TRUE(gRootNames.end() != std::find(gRootNames.begin(), gRootNames.end(), dir3));
   
   EXPECT_TRUE(gNames.end() != std::find(gNames.begin(), gNames.end(), "dir1"));
   EXPECT_TRUE(gNames.end() != std::find(gNames.begin(), gNames.end(), "dir2"));
   EXPECT_TRUE(gNames.end() != std::find(gNames.begin(), gNames.end(), "dir3"));
   EXPECT_TRUE(gNames.end() != std::find(gNames.begin(), gNames.end(), "file_1"));
   EXPECT_TRUE(gNames.end() != std::find(gNames.begin(), gNames.end(), "file_2"));
   EXPECT_TRUE(gNames.end() != std::find(gNames.begin(), gNames.end(), "file_3"));
   
}
