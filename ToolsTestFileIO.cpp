/* 
 * File:   ToolsTestFileIO.cpp
 * Author: kjell
 *
 * Created on August 15, 2013, 3:41 PM
 */

#include <random>
#include <cstdio>
#include <functional>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <g2log.hpp>
#include "ToolsTestFileIO.h"
#include "FileIO.h"
#include "StopWatch.h"


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
   struct ScopedFileCleanup {
      const std::string file;
      explicit ScopedFileCleanup(const std::string& name) : file(name) { }
      ~ScopedFileCleanup() {
         remove(file.c_str());
      }
   };
} // anonymous


TEST_F(TestFileIO, CannotOpenFileToRead) {
   auto fileRead = FileIO::ReadAsciiFileContent({"/xyz/*&%/x.y.z"});
   EXPECT_TRUE(fileRead.result.empty());
   EXPECT_FALSE(fileRead.error.empty());
   EXPECT_TRUE(fileRead.HasFailed());
}

TEST_F(TestFileIO, CanOpenFileToRead) {
   auto fileRead = FileIO::ReadAsciiFileContent({"/proc/stat"}); // fine as long as we are on *nix systems
   EXPECT_FALSE(fileRead.result.empty());
   EXPECT_TRUE(fileRead.error.empty());
   EXPECT_FALSE(fileRead.HasFailed());
}

TEST_F(TestFileIO, CannotWriteToFile) {
   auto fileWrite = FileIO::WriteAsciiFileContent({"xyz/123/proc/stat"},
   {
      "Hello World"
   });
   EXPECT_FALSE(fileWrite.result);
   EXPECT_FALSE(fileWrite.error.empty());
   EXPECT_TRUE(fileWrite.HasFailed());
}

TEST_F(TestFileIO, CanWriteToFileAndReadTheFile) {
   using namespace std;

   string filename{"/tmp/TestFileIO_"};
   filename.append(to_string(random_int(0, 1000000)))
           .append({"_"})
   .append(to_string(random_int(0, 1000000)));

   // cleanup/removing the created file when exiting
   ScopedFileCleanup cleanup{filename};

   auto fileWrite = FileIO::WriteAsciiFileContent(filename,{"Hello World"});
   EXPECT_TRUE(fileWrite.result);
   EXPECT_TRUE(fileWrite.error.empty());
   EXPECT_FALSE(fileWrite.HasFailed());

   auto fileRead = FileIO::ReadAsciiFileContent(filename);
   EXPECT_EQ("Hello World", fileRead.result);
   EXPECT_TRUE(fileRead.error.empty());
   EXPECT_FALSE(fileRead.HasFailed());
}

TEST_F(TestFileIO, FileIsNotADirectory) {
   std::string filename{"/tmp/123_456_789"};
   {
      ScopedFileCleanup cleanup{filename};
      EXPECT_FALSE(FileIO::DoesFileExist(filename));
      EXPECT_FALSE(FileIO::DoesDirectoryExist(filename));

      auto fileWrite = FileIO::WriteAsciiFileContent(filename,{"Hello World"});
      EXPECT_TRUE(fileWrite.result);
      EXPECT_TRUE(fileWrite.error.empty());
      EXPECT_FALSE(fileWrite.HasFailed());

      EXPECT_TRUE(FileIO::DoesFileExist(filename));
      EXPECT_FALSE(FileIO::DoesDirectoryExist(filename));
   }
   // RAII cleanup
   EXPECT_FALSE(FileIO::DoesFileExist(filename));

}

TEST_F(TestFileIO, DirectoryExistance) {
   std::string directory{"/tmp/some_temp_directory"};
   {
      EXPECT_FALSE(FileIO::DoesDirectoryExist(directory)) << directory;
      std::string createDir{"mkdir -p " + directory};
      EXPECT_EQ(0, system(createDir.c_str()));

      ScopedFileCleanup cleanup{directory};
      EXPECT_TRUE(FileIO::DoesFileExist(directory));
      EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));
   }
   // RAII cleanup
   EXPECT_FALSE(FileIO::DoesFileExist(directory));
   EXPECT_FALSE(FileIO::DoesDirectoryExist(directory));
}

TEST_F(TestFileIO, DirectoryReader_NotExistingDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory + "/_#Does_not+_exist"};
   EXPECT_TRUE(reader.Valid().HasFailed()) << reader.Valid().error;
}

TEST_F(TestFileIO, DirectoryReader_ExistingDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory};
   EXPECT_FALSE(reader.Valid().HasFailed()) << reader.Valid().error;
}



TEST_F(TestFileIO,  RemoveEmptyDirectories_FakeDirectoriesAreIgnored) {
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories({}).result); // invalids are ignored
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories({{""}}).result); // invalids are ignored

   std::vector<std::string> doNotExist({{}, {" "}, {"/does/not/exist"}});
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories(doNotExist).result); // invalids are ignored
   
   CreateSubDirectory("some_directory");
   std::string real = {mTestDirectory+"/"+"some_directory"};
   EXPECT_TRUE(FileIO::DoesDirectoryExist(real));
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories({{"does_not_exist"},real}).result); // the one invalid is ignored
   
}

TEST_F(TestFileIO, RemoveDirectories__ExpectNonEmptyToStay) {
   CreateSubDirectory("some_directory");
   CreateFile({mTestDirectory + "/some_directory/"}, "some_file");
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/some_directory/some_file"}));
   EXPECT_FALSE(FileIO::RemoveEmptyDirectories({{mTestDirectory + "/some_directory"}}).result);
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/some_directory/some_file"}));   
}

TEST_F(TestFileIO, CleanDirectoryOfFileContents_BogusDirectory) {
   std::vector<std::string> newDirectories;
   size_t removedFiles{0};   
   EXPECT_FALSE(FileIO::CleanDirectoryOfFileContents("", removedFiles, newDirectories).result);
   EXPECT_FALSE(FileIO::CleanDirectoryOfFileContents("/does/not/exist/", removedFiles, newDirectories).result);
}

TEST_F(TestFileIO, CleanDirectoryOfFileContents) {   
   std::vector<std::string> newDirectories;
   size_t removedFiles{0};  
   CreateSubDirectory("some_directory");
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/some_directory"}));
   EXPECT_EQ(removedFiles, 0);
   
   CreateFile(mTestDirectory, "some_file");  
   EXPECT_TRUE(FileIO::CleanDirectoryOfFileContents(mTestDirectory, removedFiles, newDirectories).result);
   EXPECT_EQ(removedFiles, 1);
   ASSERT_EQ(newDirectories.size(), 1);
   EXPECT_EQ(std::string{mTestDirectory + "/some_directory"}, newDirectories[0]);
   // directories are not removed
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/some_directory"})); 
}











// An empty directory will only contain "." and ".." which we ignores
TEST_F(TestFileIO, DirectoryReader_NoFilesInDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory};
   EXPECT_FALSE(reader.Valid().HasFailed());

   FileIO::DirectoryReader::Entry fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileIO::FileType::End);
   EXPECT_EQ(fileAndType.second, "");
}

TEST_F(TestFileIO, DirectoryReader_HasFilesInDirectory__AfterReset) {
   using namespace FileIO;

   DirectoryReader reader{mTestDirectory};
   DirectoryReader::Entry fileAndType = reader.Next();

   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");

   // We have already reached the end. This must be reset before reading successfully
   CreateFile(mTestDirectory, "some_file");
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");

   // After the reset we can find the file
   reader.Reset();
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileType::File);
   EXPECT_EQ(fileAndType.second, "some_file");


   // has reached the end again
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.second, "");
   EXPECT_EQ(fileAndType.first, FileType::End);


   CreateSubDirectory("some_directory");
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/some_directory"}));
   reader.Reset();

   fileAndType = reader.Next();
   EXPECT_NE(fileAndType.first, FileType::End);

   std::string filename;
   std::string directoryname;

   for (size_t count = 0; count < 2; ++count) {
      if (fileAndType.first == FileType::Directory) {
         directoryname = fileAndType.second;
         fileAndType = reader.Next();
      }

      if (fileAndType.first == FileType::File) {
         filename = fileAndType.second;
         reader.Next();
      }
   }

   EXPECT_EQ(filename, "some_file");
   EXPECT_EQ(directoryname, "some_directory");
   fileAndType = reader.Next();

   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");
}



TEST_F(TestFileIO, AThousandFiles) {
   using namespace FileIO;
   for (size_t index = 0; index < 1000; ++index) {
      CreateFile(mTestDirectory, std::to_string(index));
   }

   std::vector<std::string> files;
   DirectoryReader::Entry entry;

   DirectoryReader reader(mTestDirectory);
   StopWatch timeToFind;
   entry = reader.Next();
   while (entry.first != FileType::End) {
      ASSERT_NE(entry.first, FileType::Directory);
      ASSERT_NE(entry.first, FileType::Unknown);
      files.push_back(entry.second);
      entry = reader.Next();
   }

   ASSERT_EQ(files.size(), 1000);
   LOG(INFO) << "Time to find 1000 files and save them took: " << timeToFind.ElapsedUs() << " us";

   std::sort(files.begin(), files.end(), [](const std::string& lh, const std::string & rh) {
      return std::stoul(lh) < std::stoul(rh);
   });
   for (size_t index = 0; index < 1000; ++index) {
      EXPECT_EQ(files[index], std::to_string(index));
   }
}


// FileIO #files   time
//        63,8841  761 ms
//        994,080  1 sec

// Boost #files   time
//       63,8841  3199 ms
//       985,524  5 sec
TEST_F(TestFileIO, DISABLED_System_Performance_FileIO__vs_Boost) {
   using namespace FileIO;

   DirectoryReader::Entry entry;

   StopWatch timeToFind;

   size_t filecounter = 0;

   std::string path = {"/usr/local/probe/pcap"};
   DirectoryReader reader(path);
   if (false == reader.Valid().HasFailed()) {
      reader.Next();
      while (entry.first != FileIO::FileType::End) {
         ++filecounter;
      }
      entry = reader.Next();
   }

  
   LOG(INFO) << "FileIO Time to find " << filecounter << "took: " << timeToFind.ElapsedSec() << " sec";
   timeToFind.Restart();
   boost::filesystem::path boostPath = path;
   boost::filesystem::directory_iterator end;
   filecounter = 0;
   for (boost::filesystem::directory_iterator dir_iter(boostPath); dir_iter != end; ++dir_iter) {
      if (boost::filesystem::is_regular_file(dir_iter->status())) {
         ++filecounter;
      }
   }
   LOG(INFO)<< "Boost Time to find " << filecounter << "took: " << timeToFind.ElapsedSec() << " sec";
}


TEST_F(TestFileIO, DirectoryReader_NotExistingDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory + "/_#Does_not+_exist"};
   EXPECT_TRUE(reader.Valid().HasFailed()) << reader.Valid().error;
}

TEST_F(TestFileIO, DirectoryReader_ExistingDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory};
   EXPECT_FALSE(reader.Valid().HasFailed()) << reader.Valid().error;
}


// An empty directory will only contain "." and ".." which we ignores
TEST_F(TestFileIO, DirectoryReader_NoFilesInDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory};
   EXPECT_FALSE(reader.Valid().HasFailed());
   
   FileIO::DirectoryReader::Entry fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileIO::FileType::End);
   EXPECT_EQ(fileAndType.second, "");
}


TEST_F(TestFileIO, DirectoryReader_HasFilesInDirectory__AfterReset) {
   using namespace FileIO;  
   
   DirectoryReader reader{mTestDirectory};
   DirectoryReader::Entry fileAndType = reader.Next(); 
   
   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");
      
   // We have already reached the end. This must be reset before reading successfully
   CreateFile(mTestDirectory, "some_file");
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");
   
   // After the reset we can find the file
   reader.Reset();
   fileAndType = reader.Next();   
   EXPECT_EQ(fileAndType.first, FileType::File);
   EXPECT_EQ(fileAndType.second, "some_file");
  

   // has reached the end again
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.second, "");
   EXPECT_EQ(fileAndType.first, FileType::End);
   
   
   CreateSubDirectory("some_directory");
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/some_directory"}));
   reader.Reset();
   
   fileAndType = reader.Next();
   EXPECT_NE(fileAndType.first, FileType::End);
   
   std::string filename;
   std::string directoryname;

   for (size_t count = 0; count < 2; ++count) {
      if (fileAndType.first == FileType::Directory) {
         directoryname = fileAndType.second;
         fileAndType = reader.Next();
      }

      if (fileAndType.first == FileType::File) {
         filename = fileAndType.second;
         reader.Next();
      }
   }   
   
   EXPECT_EQ(filename, "some_file");
   EXPECT_EQ(directoryname, "some_directory");
   fileAndType = reader.Next();
   
   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");
}

   //   for (size_t index = 0; index < 1000; ++index) {
   //  CreateSubDirectory(std::to_string(index));      
  // }
TEST_F(TestFileIO, AThousandFiles) {
   using namespace FileIO;  
     
   for (size_t index = 0; index < 1000; ++index) {
      CreateFile(mTestDirectory, std::to_string(index));
   }
   
   
   std::vector<std::string> files;
   DirectoryReader::Entry entry;
   
   DirectoryReader reader(mTestDirectory);  
   StopWatch timeToFind;
   entry = reader.Next();
   while(entry.first != FileType::End) {
      ASSERT_NE(entry.first, FileType::Directory);
      ASSERT_NE(entry.first, FileType::Unknown);
      files.push_back(entry.second);
      entry = reader.Next();
   }
   
   ASSERT_EQ(files.size(), 1000);
   std::cout << "Time to find 1000 files and save them took: " << timeToFind.ElapsedUs() << " us" << std::endl;

   std::sort(files.begin(), files.end(), [](const std::string& lh, const std::string& rh){
      return std::stoul(lh) < std::stoul(rh);
   });
   for(size_t index = 0; index < 1000; ++index) {
      EXPECT_EQ(files[index], std::to_string(index));
   }
}

TEST_F(TestFileIO, DISABLED_System_Performance_FileIO__vs_Boost) {
   using namespace FileIO;

   DirectoryReader::Entry entry;

   StopWatch timeToFind;

   size_t filecounter = 0;

   std::string path = {"/usr/local/probe/pcap"};
   DirectoryReader reader(path);
   if (false == reader.Valid().HasFailed()) {
      reader.Next();
      while (entry.first != FileIO::FileType::End) {
         ++filecounter;
      }
      entry = reader.Next();
   }

   // 65, 4841 took: 0 se
   std::cout << "FileIO Time to find " << filecounter << "took: " << timeToFind.ElapsedSec() << " sec" << std::endl;

   timeToFind.Restart();
   boost::filesystem::path boostPath = path;
   boost::filesystem::directory_iterator end;
   filecounter = 0;
   for (boost::filesystem::directory_iterator dir_iter(boostPath); dir_iter != end; ++dir_iter) {
      if (boost::filesystem::is_regular_file(dir_iter->status())) {
         ++filecounter;
      }
   }
   std::cout << "Boost Time to find " << filecounter << "took: " << timeToFind.ElapsedSec() << " sec" << std::endl;
}
