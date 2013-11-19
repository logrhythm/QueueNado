#include "DriveInfoCommand.h"
#include "DriveInfoCommandTest.h"
#include "MockDriveInfoCommand.h"
#include "DriveInfo.h"
#include "Crowbar.h"
#include "MockConf.h"
#include "FileIO.h"
#include "Conversion.h"
#include "StringFix.h"
#include "MockDriveInfo.h"




#ifdef LR_DEBUG



TEST_F(DriveInfoCommandTest, DoesMockWork) {
   MockDriveInfoCommand testCommand(cmd,autoManagedManager);
   autoManagedManager->SetSuccess(true);
   autoManagedManager->SetResult("test result");

   MockConf conf;   
   protoMsg::CommandRequest request;
   request.set_type(::protoMsg::CommandRequest_CommandType_DRIVEINFO);
   protoMsg::CommandReply reply = testCommand.Execute(conf);
   
   EXPECT_EQ(autoManagedManager->getRunCommand(), "/sbin/parted");
   EXPECT_EQ(autoManagedManager->getRunArgs(), " -lm print");
   
   EXPECT_TRUE(reply.success());
   EXPECT_EQ(reply.result(), "test result");
   
   protoMsg::DrivesInfo drives;
   ASSERT_FALSE(drives.ParseFromString(reply.result())) << "\nRESULT: " << reply.result();
   
}

TEST_F(DriveInfoCommandTest, BuildDriveInfo) {
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.86.nodas.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   std::string parted = partedReading.result;
   
   auto devices = stringfix::explode("BYT;", parted);
   ASSERT_EQ(6, devices.size());  
   
   MockDriveInfo driveInfo("dummy");

   auto device0 = stringfix::split("\n", devices[0]);
   ASSERT_EQ(device0.size(), 3);
   
   EXPECT_TRUE(driveInfo.BuildDriveInfo(device0[0]));
   EXPECT_TRUE(driveInfo.has_model());
   EXPECT_TRUE(driveInfo.has_device());
   EXPECT_TRUE(driveInfo.has_capacityinb());
   EXPECT_TRUE(driveInfo.has_table());
   EXPECT_EQ(driveInfo.model(),"DELL PERC H710 (scsi)");
   EXPECT_EQ(driveInfo.device(),"/dev/sda");
   
   EXPECT_NE(driveInfo.capacityinb(), 299);
   size_t capacityInBytes = size_t(299) << GB_TO_B_SHIFT;
   EXPECT_EQ(driveInfo.capacityinb(), capacityInBytes) << "\n: " << (size_t(299) << GB_TO_B_SHIFT) << " bytes";
   
   EXPECT_EQ(driveInfo.table(),"msdos");
   
   // then do BuildPartitionInfo  for the two partitions on device sda
}

TEST_F(DriveInfoCommandTest, DISABLED_GetMockPartitionsFromNoDasBox) {
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.86.nodas.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   std::string parted = partedReading.result;
   
   MockDriveInfoCommand testCommand(cmd,autoManagedManager);
   std::vector<DriveInfo> allDrives;
   EXPECT_NO_THROW( allDrives = testCommand.ExtractPartedToDrives(parted));
   ASSERT_EQ(allDrives.size(), 6);
   size_t count = 0;
   for (auto& drive : allDrives) {
      EXPECT_TRUE(drive.Success()) << "\nidx:"  << count;
              ++count;
   }
   
   Conversion convert;
   // Part0
   DriveInfo d0 = allDrives[0];
   DriveInfo d0_expect{"whatever"};
   d0_expect.set_model("DELL PERC H71"); 
   d0_expect.set_device("dev/sda");
   d0_expect.set_capacityinb(convert.ReadBytesFromNBytesString("299 GB"));
   d0_expect.set_table("msdos");
   auto* part1Ptr = d0_expect.add_partitions();
   auto& part1 = *part1Ptr;
   part1.set_number(1);
   part1.set_startinb(convert.ReadBytesFromNBytesString("1049kB"));
   part1.set_endinb(convert.ReadBytesFromNBytesString("525MB"));
   part1.set_sizeinb(convert.ReadBytesFromNBytesString("524MB"));
   part1.set_type(""); // primary ignored (always when a flag is set but we ignore if for now)
   part1.set_filesystem("ext4");
   part1.set_flags("boot");
   
   auto* part2Ptr = d0_expect.add_partitions();
   auto& part2 = *part2Ptr;
   part2.set_number(2);
   part2.set_startinb(convert.ReadBytesFromNBytesString("525MB"));
   part2.set_endinb(convert.ReadBytesFromNBytesString("299GB"));
   part2.set_sizeinb(convert.ReadBytesFromNBytesString("299GB"));
   part2.set_type(""); // primary ignored (always when a flag is set but we ignore if for now)
   part2.set_filesystem("");
   part2.set_flags("lvm");
   
   EXPECT_TRUE(Compare(d0, d0_expect));
}

     
//      protoMsg::DrivesInfo allDrives;
//   for (const auto& drive : drives) {
//      protoMsg::DrivesInfo_DriveInfo* driveMsg = allDrives.add_drives();
//      *driveMsg = drive;
//   }
//   reply.set_result(allDrives.SerializeAsString());
//   
//   
//   protoMsg::DrivesInfo drives;
//   ASSERT_TRUE(drives.ParseFromString(reply.result())) << "\nRESULT: " << reply.result();



//TEST_F(DriveInfoCommandTest,GetSingleDiskParted) {
//   MockDriveInfoCommand command(cmd,autoManagedManager);
//   
//   std::string testDriveId("sde");
//   
//   EXPECT_EQ(" -s /dev/sde print", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "_";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = ".";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "/";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "\\";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "\"";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "*";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "?";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "(";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = ")";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "^";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "$";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "#";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "!";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "@";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "-";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "=";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "+";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "|";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "{";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "}";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "[";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "]";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "'";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = ":";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = ";";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "<";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = ">";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "~";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//   testDriveId = "`";
//   EXPECT_EQ("", command.GetSingleDiskPartedArgs(testDriveId));
//}
//
//TEST_F(DriveInfoCommandTest,GetValidDriveInfoFromPartedOutput) {
//   
//   std::string diskFoundPartedOutput("Model: DELL PERC H700 (scsi)\r\n\
//Disk /dev/sda: 146GB\r\n\
//Sector size (logical/physical): 512B/512B\r\n\
//Partition Table: msdos\r\n\
//\r\n\
//Number  Start   End    Size   Type     File system  Flags\r\n\
// 1      1049kB  525MB  524MB  primary  ext4         boot\r\n\
// 2      525MB   146GB  146GB  primary               lvm\r\n\
// 3      1049kB  898GB  898GB  primary  ext4\r\n\
//");
//   
//   DriveInfo validDiskInfo(diskFoundPartedOutput);
//   EXPECT_TRUE(validDiskInfo.device() == "/dev/sda");
//   EXPECT_TRUE(validDiskInfo.model() == "DELL PERC H700 (scsi)");
//   EXPECT_TRUE(validDiskInfo.capacityinb() == (146L<<GB_TO_B_SHIFT));
//   EXPECT_TRUE(validDiskInfo.table() == "msdos");
//   ASSERT_TRUE(validDiskInfo.partitions_size() == 3);
//   EXPECT_TRUE(validDiskInfo.partitions(0).number() == 1);
//   EXPECT_TRUE(validDiskInfo.partitions(1).number() == 2);
//   EXPECT_EQ(1049<<KB_TO_B_SHIFT,validDiskInfo.partitions(0).startinb());
//   EXPECT_TRUE(validDiskInfo.partitions(1).startinb() == (525L<<MB_TO_B_SHIFT));
//   EXPECT_TRUE(validDiskInfo.partitions(0).endinb() == (525L<<MB_TO_B_SHIFT));
//   EXPECT_TRUE(validDiskInfo.partitions(1).endinb() == (146L<<GB_TO_B_SHIFT));
//   EXPECT_TRUE(validDiskInfo.partitions(0).sizeinb() == (524L<<MB_TO_B_SHIFT));
//   EXPECT_TRUE(validDiskInfo.partitions(1).sizeinb() == (146L<<GB_TO_B_SHIFT));   
//   EXPECT_TRUE(validDiskInfo.partitions(0).type() == "primary");
//   EXPECT_TRUE(validDiskInfo.partitions(1).type() == "primary");   
//   EXPECT_TRUE(validDiskInfo.partitions(0).filesystem() == "ext4");
//   EXPECT_TRUE(validDiskInfo.partitions(1).filesystem() == ""); 
//   EXPECT_TRUE(validDiskInfo.partitions(2).filesystem() == "ext4"); 
//   EXPECT_TRUE(validDiskInfo.partitions(0).flags() == "boot");
//   EXPECT_TRUE(validDiskInfo.partitions(1).flags() == "lvm"); 
//   EXPECT_TRUE(validDiskInfo.partitions(2).flags() == ""); 
//   
//   
//}
//TEST_F(DriveInfoCommandTest,GetValidDriveInfoFromPartedOutputDAS) {
//   
//   std::string diskFoundPartedOutput("Model: DELL PERC H810 (scsi)\r\n\
//Disk /dev/sdc: 26.4TB\r\n\
//Sector size (logical/physical): 512B/512B\r\n\
//Partition Table: gpt\r\n\
//\r\n\
//Number  Start   End     Size    File system  Name     Flags\r\n\
// 1      1049kB  13.2TB  13.2TB  ext4         primary\r\n\
// 2      13.2TB  26.4TB  13.2TB  ext4         primary\r\n\
//\r\n\
//\r\n\
//");
//   
//   DriveInfo validDiskInfo(diskFoundPartedOutput);
//   EXPECT_TRUE(validDiskInfo.device() == "/dev/sdc");
//   EXPECT_TRUE(validDiskInfo.model() == "DELL PERC H810 (scsi)");
//   EXPECT_EQ(validDiskInfo.capacityinb(),(26L<<TB_TO_B_SHIFT) + 400*(1L<<GB_TO_B_SHIFT));
//   EXPECT_TRUE(validDiskInfo.table() == "gpt");
//   ASSERT_TRUE(validDiskInfo.partitions_size() == 2);
//   EXPECT_TRUE(validDiskInfo.partitions(0).number() == 1);
//   EXPECT_TRUE(validDiskInfo.partitions(1).number() == 2);
//   EXPECT_EQ(1049L<<KB_TO_B_SHIFT,validDiskInfo.partitions(0).startinb());
//   EXPECT_EQ(validDiskInfo.partitions(1).startinb(),(13L<<TB_TO_B_SHIFT)+ 200*(1L<<GB_TO_B_SHIFT));
//   EXPECT_EQ(validDiskInfo.partitions(0).endinb(),(13L<<TB_TO_B_SHIFT)+ 200*(1L<<GB_TO_B_SHIFT));
//   EXPECT_EQ(validDiskInfo.partitions(1).endinb(),(26L<<TB_TO_B_SHIFT)+ 400*(1L<<GB_TO_B_SHIFT));
//   EXPECT_EQ(validDiskInfo.partitions(0).sizeinb(),(13L<<TB_TO_B_SHIFT)+200*(1L<<GB_TO_B_SHIFT));   
//   EXPECT_EQ(validDiskInfo.partitions(1).sizeinb(),(13L<<TB_TO_B_SHIFT)+ 200*(1L<<GB_TO_B_SHIFT));   
//   EXPECT_TRUE(validDiskInfo.partitions(0).type() == "primary");
//   EXPECT_TRUE(validDiskInfo.partitions(1).type() == "primary");   
//   EXPECT_TRUE(validDiskInfo.partitions(0).filesystem() == "ext4");
//   EXPECT_TRUE(validDiskInfo.partitions(1).filesystem() == "ext4"); 
//   EXPECT_TRUE(validDiskInfo.partitions(0).flags() == "");
//   EXPECT_TRUE(validDiskInfo.partitions(1).flags() == ""); 
//   
//   
//}
//
//TEST_F(DriveInfoCommandTest,GetEmptyDriveInfoFromPartedOutput) {
//   std::string diskNotFoundPartedOutput("Error: Could not stat device /dev/sdf - No such file or directory.");
//   DriveInfo invalidDiskInfo(diskNotFoundPartedOutput);
//   EXPECT_TRUE(invalidDiskInfo.device() == "");
//   EXPECT_TRUE(invalidDiskInfo.model() == "");
//   EXPECT_TRUE(invalidDiskInfo.capacityinb() == 0);
//   ASSERT_TRUE(invalidDiskInfo.partitions_size() == 0);
//
//   
//}
//
//TEST_F(DriveInfoCommandTest, DISABLED_TestToGetSomething) {
//   protoMsg::CommandRequest request;
//   MockConf conf;
//   
//   request.set_type(::protoMsg::CommandRequest_CommandType_DRIVEINFO);
//   
//   std::shared_ptr<Command> testCommand = DriveInfoCommand::Construct(request);
//   
//   protoMsg::CommandReply reply = testCommand->Execute(conf);
//   
//   ASSERT_TRUE(reply.success());
//   
//   protoMsg::DrivesInfo drives;
//   ASSERT_TRUE(drives.ParseFromString(reply.result()));
//}
///**
// * This test requires a running manager
// * @param 
// */
//TEST_F(DriveInfoCommandTest, DISABLED_TestWithSocket) {
// 
//   protoMsg::CommandRequest request;
//   request.set_type(::protoMsg::CommandRequest_CommandType_DRIVEINFO);
//   Crowbar sender("tcp://127.0.0.1:5556");
//   ASSERT_TRUE(sender.Wield());
//   
//   std::vector<std::string> swings;
//   std::vector<std::string> results;
//   
//   swings.emplace_back(request.SerializeAsString());
//   
//   ASSERT_TRUE(sender.Flurry(swings));
//   ASSERT_TRUE(sender.BlockForKill(results));
//   protoMsg::CommandReply reply;
//   ASSERT_TRUE(!results.empty());
//   reply.ParseFromString(results[0]);
//   ASSERT_TRUE(reply.success());
//   
//   protoMsg::DrivesInfo drives;
//   ASSERT_TRUE(drives.ParseFromString(reply.result()));
//   
//}
//#else 
//
//TEST_F(DriveInfoCommandTest, NullTest) {
//   EXPECT_TRUE(true);
//}
//
#endif