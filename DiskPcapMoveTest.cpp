#include "DiskPcapMoveTest.h"
#include "MockConf.h"
#include "DiskPcapMover.h"
#include "MockDiskPcapMover.h"

TEST_F(DiskPcapMoveTest, FirstTestWillFail) {

   MockConf conf;
   DiskPcapMover mover(conf);
   EXPECT_FALSE(mover.IsPcapStorageValid("/blah/DoesNotExist/123/"));
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


TEST_F(DiskPcapMoveTest, DoesDirectoryHaveContent_CheckFile) {
   MockConf conf;
   MockDiskPcapMover mover(conf);
   
   EXPECT_FALSE(mover.DoesDirectoryHaveContent(mTestDirectory));
   CreateFile(mTestDirectory, "some_file");  
   EXPECT_TRUE(mover.DoesDirectoryHaveContent(mTestDirectory));
}