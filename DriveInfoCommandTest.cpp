#include "DriveInfoCommand.h"
#include "DriveInfoCommandTest.h"
#include "MockDriveInfoCommand.h"
#include "DriveInfo.h"
#include "Crowbar.h"
#include "MockConf.h"

#ifdef LR_DEBUG
TEST_F(DriveInfoCommandTest,GetSingleDiskParted) {
   MockDriveInfoCommand command(cmd,autoManagedManager);
   
   std::string testDriveId("sde");
   
   EXPECT_EQ(" -s /dev/sde print", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "_";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = ".";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "/";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "\\";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "\"";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "*";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "?";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "(";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = ")";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "^";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "$";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "#";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "!";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "@";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "-";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "=";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "+";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "|";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "{";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "}";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "[";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "]";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "'";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = ":";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = ";";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "<";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = ">";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "~";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
   testDriveId = "`";
   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
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
 3      1049kB  898GB  898GB  primary  ext4\r\n\
");
   
   DriveInfo validDiskInfo(diskFoundPartedOutput);
   EXPECT_TRUE(validDiskInfo.device() == "/dev/sda");
   EXPECT_TRUE(validDiskInfo.model() == "DELL PERC H700 (scsi)");
   EXPECT_TRUE(validDiskInfo.capacityinb() == (146L<<GB_TO_B_SHIFT));
   EXPECT_TRUE(validDiskInfo.table() == "msdos");
   ASSERT_TRUE(validDiskInfo.partitions_size() == 3);
   EXPECT_TRUE(validDiskInfo.partitions(0).number() == 1);
   EXPECT_TRUE(validDiskInfo.partitions(1).number() == 2);
   EXPECT_EQ(1049<<KB_TO_B_SHIFT,validDiskInfo.partitions(0).startinb());
   EXPECT_TRUE(validDiskInfo.partitions(1).startinb() == (525L<<MB_TO_B_SHIFT));
   EXPECT_TRUE(validDiskInfo.partitions(0).endinb() == (525L<<MB_TO_B_SHIFT));
   EXPECT_TRUE(validDiskInfo.partitions(1).endinb() == (146L<<GB_TO_B_SHIFT));
   EXPECT_TRUE(validDiskInfo.partitions(0).sizeinb() == (524L<<MB_TO_B_SHIFT));
   EXPECT_TRUE(validDiskInfo.partitions(1).sizeinb() == (146L<<GB_TO_B_SHIFT));   
   EXPECT_TRUE(validDiskInfo.partitions(0).type() == "primary");
   EXPECT_TRUE(validDiskInfo.partitions(1).type() == "primary");   
   EXPECT_TRUE(validDiskInfo.partitions(0).filesystem() == "ext4");
   EXPECT_TRUE(validDiskInfo.partitions(1).filesystem() == ""); 
   EXPECT_TRUE(validDiskInfo.partitions(2).filesystem() == "ext4"); 
   EXPECT_TRUE(validDiskInfo.partitions(0).flags() == "boot");
   EXPECT_TRUE(validDiskInfo.partitions(1).flags() == "lvm"); 
   EXPECT_TRUE(validDiskInfo.partitions(2).flags() == ""); 
   
   
}
TEST_F(DriveInfoCommandTest,GetValidDriveInfoFromPartedOutputDAS) {
   
   std::string diskFoundPartedOutput("Model: DELL PERC H810 (scsi)\r\n\
Disk /dev/sdc: 26.4TB\r\n\
Sector size (logical/physical): 512B/512B\r\n\
Partition Table: gpt\r\n\
\r\n\
Number  Start   End     Size    File system  Name     Flags\r\n\
 1      1049kB  13.2TB  13.2TB  ext4         primary\r\n\
 2      13.2TB  26.4TB  13.2TB  ext4         primary\r\n\
\r\n\
\r\n\
");
   
   DriveInfo validDiskInfo(diskFoundPartedOutput);
   EXPECT_TRUE(validDiskInfo.device() == "/dev/sdc");
   EXPECT_TRUE(validDiskInfo.model() == "DELL PERC H810 (scsi)");
   EXPECT_EQ(validDiskInfo.capacityinb(),(26L<<TB_TO_B_SHIFT) + 400*(1L<<GB_TO_B_SHIFT));
   EXPECT_TRUE(validDiskInfo.table() == "gpt");
   ASSERT_TRUE(validDiskInfo.partitions_size() == 2);
   EXPECT_TRUE(validDiskInfo.partitions(0).number() == 1);
   EXPECT_TRUE(validDiskInfo.partitions(1).number() == 2);
   EXPECT_EQ(1049L<<KB_TO_B_SHIFT,validDiskInfo.partitions(0).startinb());
   EXPECT_EQ(validDiskInfo.partitions(1).startinb(),(13L<<TB_TO_B_SHIFT)+ 200*(1L<<GB_TO_B_SHIFT));
   EXPECT_EQ(validDiskInfo.partitions(0).endinb(),(13L<<TB_TO_B_SHIFT)+ 200*(1L<<GB_TO_B_SHIFT));
   EXPECT_EQ(validDiskInfo.partitions(1).endinb(),(26L<<TB_TO_B_SHIFT)+ 400*(1L<<GB_TO_B_SHIFT));
   EXPECT_EQ(validDiskInfo.partitions(0).sizeinb(),(13L<<TB_TO_B_SHIFT)+200*(1L<<GB_TO_B_SHIFT));   
   EXPECT_EQ(validDiskInfo.partitions(1).sizeinb(),(13L<<TB_TO_B_SHIFT)+ 200*(1L<<GB_TO_B_SHIFT));   
   EXPECT_TRUE(validDiskInfo.partitions(0).type() == "primary");
   EXPECT_TRUE(validDiskInfo.partitions(1).type() == "primary");   
   EXPECT_TRUE(validDiskInfo.partitions(0).filesystem() == "ext4");
   EXPECT_TRUE(validDiskInfo.partitions(1).filesystem() == "ext4"); 
   EXPECT_TRUE(validDiskInfo.partitions(0).flags() == "");
   EXPECT_TRUE(validDiskInfo.partitions(1).flags() == ""); 
   
   
}

TEST_F(DriveInfoCommandTest,GetEmptyDriveInfoFromPartedOutput) {
   std::string diskNotFoundPartedOutput("Error: Could not stat device /dev/sdf - No such file or directory.");
   DriveInfo invalidDiskInfo(diskNotFoundPartedOutput);
   EXPECT_TRUE(invalidDiskInfo.device() == "");
   EXPECT_TRUE(invalidDiskInfo.model() == "");
   EXPECT_TRUE(invalidDiskInfo.capacityinb() == 0);
   ASSERT_TRUE(invalidDiskInfo.partitions_size() == 0);

   
}

TEST_F(DriveInfoCommandTest, DISABLED_TestToGetSomething) {
   protoMsg::CommandRequest request;
   MockConf conf;
   
   request.set_type(::protoMsg::CommandRequest_CommandType_DRIVEINFO);
   
   std::shared_ptr<Command> testCommand = DriveInfoCommand::Construct(request);
   
   protoMsg::CommandReply reply = testCommand->Execute(conf);
   
   ASSERT_TRUE(reply.success());
   
   protoMsg::DrivesInfo drives;
   ASSERT_TRUE(drives.ParseFromString(reply.result()));
}
/**
 * This test requires a running manager
 * @param 
 */
TEST_F(DriveInfoCommandTest, TestWithSocket) {
 
   protoMsg::CommandRequest request;
   request.set_type(::protoMsg::CommandRequest_CommandType_DRIVEINFO);
   Crowbar sender("tcp://127.0.0.1:5556");
   ASSERT_TRUE(sender.Wield());
   
   std::vector<std::string> swings;
   std::vector<std::string> results;
   
   swings.emplace_back(request.SerializeAsString());
   
   ASSERT_TRUE(sender.Flurry(swings));
   ASSERT_TRUE(sender.BlockForKill(results));
   protoMsg::CommandReply reply;
   ASSERT_TRUE(!results.empty());
   reply.ParseFromString(results[0]);
   ASSERT_TRUE(reply.success());
   
   protoMsg::DrivesInfo drives;
   ASSERT_TRUE(drives.ParseFromString(reply.result()));
   
}
#else 

TEST_F(DriveInfoCommandTest, NullTest) {
   EXPECT_TRUE(true);
}

#endif