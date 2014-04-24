#include "DiskPcapMoveTest.h"
#include "MockConf.h"
#include "DiskPcapMover.h"
#include "MockDiskPcapMover.h"
#include "FileIO.h"
#include "MockConfSlave.h"
#include <vector>
#include <g2log.hpp>
#include <sys/stat.h>

#ifdef LR_DEBUG

TEST_F(DiskPcapMoveTest, GetPcapCaptureFolderPerPartitionLimit) {
   MockConfSlave slave;
   slave.mConfLocation = "resources/test.yaml.MovePcapFiles_Valid";
   Conf conf1 = slave.GetConf();
   EXPECT_EQ(1, conf1.GetPcapCaptureFolderPerPartitionLimit());
   
   slave.mConfLocation = "resources.test.yaml.MovePcapFiles_Invalid";
   Conf conf2 = slave.GetConf();
   EXPECT_EQ(16000, conf2.GetPcapCaptureFolderPerPartitionLimit());
}


TEST_F(DiskPcapMoveTest, DirectoryExistsUpToLimit) {
   MockConf conf;
   conf.mPcapCaptureFolderPerPartitionLimit = 0; // crazy value with zero. This funky stuff is prevented in the Conf
   
   MockDiskPcapMover mover(conf);
   EXPECT_TRUE(mover.DirectoryExistsUpToLimit(mTestDirectory)); 
   
   
   conf.mPcapCaptureFolderPerPartitionLimit = 1;
   EXPECT_FALSE(mover.DirectoryExistsUpToLimit(mTestDirectory)); 

   CreateSubDirectory("1");
   EXPECT_FALSE(mover.DirectoryExistsUpToLimit(mTestDirectory));  // wrong start number
   
   conf.mPcapCaptureFolderPerPartitionLimit = 2;
   CreateSubDirectory("0");
   EXPECT_TRUE(mover.DirectoryExistsUpToLimit(mTestDirectory));  
}





TEST_F(DiskPcapMoveTest, DirectoryNoOverflow) {
   MockConf conf;
   conf.mPcapCaptureFolderPerPartitionLimit = 0;
   
   MockDiskPcapMover mover(conf);
   EXPECT_TRUE(mover.DirectoryNoOverflow(mTestDirectory)); 
      
   CreateSubDirectory("0");
   EXPECT_FALSE(mover.DirectoryNoOverflow(mTestDirectory)); 
   
   conf.mPcapCaptureFolderPerPartitionLimit = 1;
   EXPECT_TRUE(mover.DirectoryNoOverflow(mTestDirectory)); 

   CreateSubDirectory("2");
   EXPECT_FALSE(mover.DirectoryNoOverflow(mTestDirectory)); 

   conf.mPcapCaptureFolderPerPartitionLimit = 2;
   EXPECT_FALSE(mover.DirectoryNoOverflow(mTestDirectory)); 

   CreateSubDirectory("1");
   EXPECT_FALSE(mover.DirectoryNoOverflow(mTestDirectory)); 

   
   CreateSubDirectory("15999");
   EXPECT_FALSE(mover.DirectoryNoOverflow(mTestDirectory)); 
   conf.mPcapCaptureFolderPerPartitionLimit = 16000;
   EXPECT_TRUE(mover.DirectoryNoOverflow(mTestDirectory)); 
}

TEST_F(DiskPcapMoveTest, DoesDirectoryHaveUnhashedFiles) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 10;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);

   
   mover.CreatePcapSubDirectories(mTestDirectory);
   EXPECT_TRUE(mover.HasNoUnhashedFiles(mTestDirectory));
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/9/"}));
   CreateFile({mTestDirectory + "/9/"}, "some_file_1");
   EXPECT_TRUE(mover.HasNoUnhashedFiles(mTestDirectory));

   // add unhashed file   
   CreateFile(mTestDirectory, "some_file_2");
   EXPECT_FALSE(mover.HasNoUnhashedFiles(mTestDirectory));
}

TEST_F(DiskPcapMoveTest, DoesDirectoryHaveContent_CheckFile) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   
   EXPECT_FALSE(mover.DoesDirectoryHaveContent(mTestDirectory));
   CreateFile(mTestDirectory, "some_file");  
   EXPECT_TRUE(mover.DoesDirectoryHaveContent(mTestDirectory));
}


TEST_F(DiskPcapMoveTest, CreatePcapSubDirectories_FailForBogusLocation){
   MockConf conf;
   MockDiskPcapMover mover(conf);
   EXPECT_FALSE(mover.CreatePcapSubDirectories({"does/not/exist"}));
}



TEST_F(DiskPcapMoveTest, FixupPermissions__SanityCheck) {
   auto dir = CreateSubDirectory("testing");
   EXPECT_TRUE(FileIO::DoesDirectoryExist(dir));
   
   MockConf conf;
   MockDiskPcapMover mover(conf);
   const mode_t permissions = mover.GetPcapPermissions();
   int changed = chmod(dir.c_str(), permissions);
   
   struct stat buf;
   EXPECT_TRUE(0 == stat(dir.c_str(), &buf));
   EXPECT_EQ(permissions, (buf.st_mode & permissions));
   
   EXPECT_NE(permissions, (buf.st_mode & 777));
}


TEST_F(DiskPcapMoveTest, FixupPermissions) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 10;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);
   mover.CreatePcapSubDirectories(mTestDirectory);

   
  
   
   const mode_t permissions = mover.GetPcapPermissions();
   EXPECT_EQ(permissions, mode_t{(S_IRWXU | S_IXGRP | S_IXOTH | S_IROTH)});
   
   for (size_t index = 0; index < 10; ++index) {
      const std::string directory = {mTestDirectory + "/" + std::to_string(index)};
      EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));

      struct stat buf;
      EXPECT_TRUE(0 == stat(directory.c_str(), &buf));
      EXPECT_EQ(permissions, buf.st_mode & permissions);

      // as a last step. set the permissions to something ELSE
      // and verify that they are  now false
      EXPECT_TRUE(0 == chmod(directory.c_str(), 777)) << "error: " << std::strerror(errno);
      EXPECT_TRUE(0 == stat(directory.c_str(), &buf));
      EXPECT_NE(permissions, buf.st_mode & permissions);
   }

   // Verify that they are fixed back again at starup
   mover.FixupPermissions(mTestDirectory);
   for (size_t index = 0; index < 10; ++index) {
      const std::string directory = {mTestDirectory + "/" + std::to_string(index)};
      EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));

      const mode_t permissions = mover.GetPcapPermissions();
      struct stat buf;
      EXPECT_TRUE(0 == stat(directory.c_str(), &buf));
      EXPECT_EQ(permissions, buf.st_mode & permissions);
   }
}



// 
// Here starts High Level Pcap Storage validation, cleanup and setup tests
// 
TEST_F(DiskPcapMoveTest, IsPcapStorageValid__Invalid) {

   MockConf conf;
   MockDiskPcapMover mover(conf);
   EXPECT_FALSE(mover.IsPcapStorageValid("/blah/DoesNotExist/123/"));
}


TEST_F(DiskPcapMoveTest, IsPcapStorageValid) {

   MockConf conf;
   MockDiskPcapMover mover(conf);
   conf.mPcapCaptureFolderPerPartitionLimit = 10;
   
   for (size_t index = 0; index < (conf.mPcapCaptureFolderPerPartitionLimit -1); ++index) {
      CreateSubDirectory(std::to_string(index));
   }
   EXPECT_FALSE(mover.IsPcapStorageValid(mTestDirectory));
   
   // Only exactly the right configuration will be accepted as valid
   CreateSubDirectory(std::to_string(9));
   EXPECT_TRUE(mover.IsPcapStorageValid(mTestDirectory));
   
   CreateSubDirectory(std::to_string(11));
   EXPECT_FALSE(mover.IsPcapStorageValid(mTestDirectory));
}


TEST_F(DiskPcapMoveTest, IsPcapStorageValid_loose_files) {

   MockConf conf;
   MockDiskPcapMover mover(conf);
   conf.mPcapCaptureFolderPerPartitionLimit = 10;
   
   for (size_t index = 0; index < (conf.mPcapCaptureFolderPerPartitionLimit); ++index) {
      CreateSubDirectory(std::to_string(index));
   }
   EXPECT_TRUE(mover.IsPcapStorageValid(mTestDirectory));
   
   CreateFile(mTestDirectory, "some_file");
   EXPECT_FALSE(mover.IsPcapStorageValid(mTestDirectory));
}

TEST_F(DiskPcapMoveTest, DeleteOldPcapStorage) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 10;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);
   //   // Only do this test if the setup is OK. Otherwise it would be 
   //   // really, really, really bad
   ASSERT_EQ(mover.GetOriginalProbePcapLocation(), mTestDirectory);
   ASSERT_EQ(conf.GetFirstPcapCaptureLocation(), mTestDirectory);
   ASSERT_EQ(conf.GetPcapCaptureLocations().size(), 1);
   LOG(INFO) << "TEST_F(DiskPcapMoveTest, DeleteOldPcapStorage)";
   EXPECT_TRUE(mover.DeleteOldPcapStorage());
   
   CreateFile(mTestDirectory, "some_file_1");
   CreateSubDirectory("some_directory");
   CreateFile({mTestDirectory + "/some_directory"}, "some_file_2");
   EXPECT_TRUE(mover.DeleteOldPcapStorage());
}


TEST_F(DiskPcapMoveTest, IsPcapStorageSetupAlready) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 10;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);
   
   EXPECT_FALSE(mover.IsPcapStorageSetupAlready());
   for (size_t index = 0; index < 9; ++index) {
      CreateSubDirectory(std::to_string(index));
      EXPECT_FALSE(mover.IsPcapStorageSetupAlready());
   }
   CreateSubDirectory(std::to_string(9));
   EXPECT_TRUE(mover.IsPcapStorageSetupAlready());
}

TEST_F(DiskPcapMoveTest,  CreatePcapStorage) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 10;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);
   
  EXPECT_FALSE(mover.IsPcapStorageSetupAlready());
  EXPECT_TRUE(mover.CreatePcapStorage());
  EXPECT_TRUE(mover.IsPcapStorageSetupAlready());   
}

TEST_F(DiskPcapMoveTest, HashContentsToPcapBuckets) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 2;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);
   
   EXPECT_TRUE(mover.CreatePcapStorage());
   // nothing to do
   for (const auto& location: conf.GetPcapCaptureLocations()) {
      ASSERT_TRUE(location.find("/tmp/") != std::string::npos);
      EXPECT_TRUE(mover.HashContentsToPcapBuckets(location)) << location;  
   }
   
   CreateFile(mTestDirectory, "553c367a-f638-457c-9916-624e189702ef"); 
   CreateFile(mTestDirectory, "aa681de2-d32b-4aa4-abe0-5b57d47da5de"); 
   for (const auto& location: conf.GetPcapCaptureLocations()) {
      ASSERT_TRUE(location.find("/tmp/") != std::string::npos);
      EXPECT_TRUE(mover.HashContentsToPcapBuckets(location)) << location;  
   }
   
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/0/553c367a-f638-457c-9916-624e189702ef"}));
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/1/aa681de2-d32b-4aa4-abe0-5b57d47da5de"}));  

   // All is already hashed. Nothing to do
   for (const auto& location: conf.GetPcapCaptureLocations()) {
      ASSERT_TRUE(location.find("/tmp/") != std::string::npos);
      EXPECT_TRUE(mover.HashContentsToPcapBuckets(location)) << location;  
   }
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/0/553c367a-f638-457c-9916-624e189702ef"}));
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/1/aa681de2-d32b-4aa4-abe0-5b57d47da5de"}));  
}



TEST_F(DiskPcapMoveTest,TheWholeShebang__NothingCreated) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 2;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);
   
   // scenario 1: empty with the directories
   EXPECT_FALSE(mover.IsPcapStorageSetupAlready());
   EXPECT_TRUE(mover.CreatePcapStorage());
   EXPECT_TRUE(mover.IsPcapStorageSetupAlready());
}

TEST_F(DiskPcapMoveTest,TheWholeShebang__FilesButNoDirectories) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   mover.mOverrideProbePcapOriginalLocation = true; // using GetFirstPcapCaptureLocation from MockConf

   conf.mPcapCaptureFolderPerPartitionLimit = 2;
   conf.mOverrideGetPcapCaptureLocations = true;
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(mTestDirectory);
   
   // scenario 1: empty with the directories
   EXPECT_FALSE(mover.IsPcapStorageSetupAlready());
   EXPECT_TRUE(mover.CreatePcapStorage());
   
   CreateFile(mTestDirectory, "553c367a-f638-457c-9916-624e189702ef"); 
   CreateFile(mTestDirectory, "aa681de2-d32b-4aa4-abe0-5b57d47da5de"); 
   EXPECT_FALSE(mover.IsPcapStorageSetupAlready());
   EXPECT_TRUE(mover.DiskPcapMover::FixUpPcapStorage());
   EXPECT_TRUE(mover.IsPcapStorageSetupAlready());
}
#endif // LR_DEBUG