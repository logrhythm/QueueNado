#include "DriveInfoCommand.h"
#include "DriveInfoCommandTest.h"
#include "MockDriveInfoCommand.h"
#include "DriveInfo.h"

#ifdef LR_DEBUG
TEST_F(DriveInfoCommandTest,GetSingleDiskParted) {
   MockDriveInfoCommand command(cmd,autoManagedManager);
   
   std::string testDriveId("sde");
   
   EXPECT_EQ("/sbin/parted -s /dev/sde print", command.GetSingleDiskParted(testDriveId));
   testDriveId = "_";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = ".";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "/";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "\\";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "\"";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "*";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "?";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "(";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = ")";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "^";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "$";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "#";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "!";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "@";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "-";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "=";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "+";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "|";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "{";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "}";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "[";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "]";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "'";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = ":";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = ";";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "<";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = ">";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "~";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
   testDriveId = "`";
   EXPECT_EQ("", command.GetSingleDiskParted(testDriveId));
}

TEST_F(DriveInfoCommandTest,GetValidDriveInfoFromPartedOutput) {
   
   std::string diskFoundPartedOutput("Model: DELL PERC H700 (scsi)\r\n\
Disk /dev/sda: 146GB\r\n\
Sector size (logical/physical): 512B/512B\r\n\
Partition Table: msdos\r\n\
\r\n\
Number  Start   End    Size   Type     File system  Flags\r\n\
 1      1049kB  525MB  524MB  primary  ext4         boot\r\n\
 2      525MB   146GB  146GB  primary               lvm\r\n\
");
   
   DriveInfo validDiskInfo(diskFoundPartedOutput);
   EXPECT_TRUE(validDiskInfo.mDevice == "/dev/sda");
   EXPECT_TRUE(validDiskInfo.mModel == "DELL PERC H700 (scsi)");
   EXPECT_TRUE(validDiskInfo.mTotalCapacityInMB == (146<<GB_TO_MB_SHIFT));
   ASSERT_TRUE(validDiskInfo.mPartitions.size() == 2);
   EXPECT_TRUE(validDiskInfo.mPartitions[0].mNumber == 1);
   EXPECT_TRUE(validDiskInfo.mPartitions[1].mNumber == 2);
   EXPECT_TRUE(validDiskInfo.mPartitions[0].mStartInKb == 1049);
   EXPECT_TRUE(validDiskInfo.mPartitions[1].mStartInKb == (525<<MB_TO_KB_SHIFT));
   EXPECT_TRUE(validDiskInfo.mPartitions[0].mEndInKb == (525<<MB_TO_KB_SHIFT));
   EXPECT_TRUE(validDiskInfo.mPartitions[1].mEndInKb == (146<<GB_TO_KB_SHIFT));
   EXPECT_TRUE(validDiskInfo.mPartitions[0].mSizeInKb == (524<<MB_TO_KB_SHIFT));
   EXPECT_TRUE(validDiskInfo.mPartitions[1].mSizeInKb == (146<<GB_TO_KB_SHIFT));   
   EXPECT_TRUE(validDiskInfo.mPartitions[0].mType == "primary");
   EXPECT_TRUE(validDiskInfo.mPartitions[1].mType == "primary");   
   EXPECT_TRUE(validDiskInfo.mPartitions[0].mFileSystem == "ext4");
   EXPECT_TRUE(validDiskInfo.mPartitions[1].mFileSystem == ""); 
   EXPECT_TRUE(validDiskInfo.mPartitions[0].mFlags == "boot");
   EXPECT_TRUE(validDiskInfo.mPartitions[1].mFlags == "lvm"); 
}

TEST_F(DriveInfoCommandTest,GetEmptyDriveInfoFromPartedOutput) {
   std::string diskNotFoundPartedOutput("Error: Could not stat device /dev/sdf - No such file or directory.");
   DriveInfo invalidDiskInfo(diskNotFoundPartedOutput);
   EXPECT_TRUE(invalidDiskInfo.mDevice == "");
   EXPECT_TRUE(invalidDiskInfo.mModel == "");
   EXPECT_TRUE(invalidDiskInfo.mTotalCapacityInMB == 0);
   ASSERT_TRUE(invalidDiskInfo.mPartitions.size() == 0);

   
}
#else 

TEST_F(DriveInfoCommandTest, NullTest) {
   EXPECT_TRUE(true);
}

#endif