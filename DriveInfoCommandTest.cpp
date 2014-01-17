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
   MockDriveInfoCommand testCommand(cmd, autoManagedManager);
   autoManagedManager.SetSuccess(true);
   autoManagedManager.SetResult("test result");

   MockConf conf;
   protoMsg::CommandRequest request;
   request.set_type(::protoMsg::CommandRequest_CommandType_DRIVEINFO);
   protoMsg::CommandReply reply = testCommand.Execute(conf);

   EXPECT_EQ(autoManagedManager.getRunCommand(), "/usr/local/probe/sbin/parted");
   EXPECT_EQ(autoManagedManager.getRunArgs(), " -lm print");

   EXPECT_FALSE(reply.success());
   EXPECT_EQ(reply.result(), "Failed to parse successfully from parted output.\nResult was:\ntest result");

   protoMsg::DrivesInfo drives;
   ASSERT_FALSE(drives.ParseFromString(reply.result())) << "\nRESULT: " << reply.result();

}
TEST_F(DriveInfoCommandTest, FailedWithBuildDriveInfo) {
   MockDriveInfo driveInfo("dummy");
   EXPECT_TRUE(driveInfo.BuildDriveInfo("0:1:2:3:4:5:6;"));
   EXPECT_TRUE(driveInfo.BuildDriveInfo("0:1:2:3:4:5:6"));

   EXPECT_FALSE(driveInfo.BuildDriveInfo("0:1:2:3:4:5:6:7;"));
   EXPECT_FALSE(driveInfo.BuildDriveInfo("0:1:2:3:4:5:6:7"));
}
TEST_F(DriveInfoCommandTest, BuildDriveInfo) {
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.86.nodas.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   std::string parted = partedReading.result;

   auto devices = stringfix::explode("BYT;", parted);
   ASSERT_EQ(7, devices.size());
   ASSERT_TRUE(devices[0].empty());
   MockDriveInfo driveInfo("dummy");

   auto device0 = stringfix::split("\n", devices[1]); // skip the empty pre 'BYT;'
   ASSERT_EQ(device0.size(), 3);

   EXPECT_TRUE(driveInfo.BuildDriveInfo(device0[0]));
   EXPECT_TRUE(driveInfo.has_model());
   EXPECT_TRUE(driveInfo.has_device());
   EXPECT_TRUE(driveInfo.has_capacityinb());
   EXPECT_TRUE(driveInfo.has_table());
   EXPECT_EQ(driveInfo.model(), "DELL PERC H710 (scsi)") << "\n\tString was: " << device0[1];
   EXPECT_EQ(driveInfo.device(), "/dev/sda");

   EXPECT_NE(driveInfo.capacityinb(), 299);
   size_t capacityInBytes = size_t(299) << GB_TO_B_SHIFT;
   EXPECT_EQ(driveInfo.capacityinb(), capacityInBytes) << "\n: " << (size_t(299) << GB_TO_B_SHIFT) << " bytes";

   EXPECT_EQ(driveInfo.table(), "msdos");

   // then do BuildPartitionInfo  for the two partitions on device sda
}
TEST_F(DriveInfoCommandTest, FailedWithBuildPartitionInfo) {
   MockDriveInfo driveInfo("dummy");

   // one flag
   EXPECT_TRUE(driveInfo.BuildPartitionInfo("1:23MB:32MB:150GB:ext4::boot;"));
   EXPECT_TRUE(driveInfo.BuildPartitionInfo("0:1:2:3:4::5;"));


   // two flags
   EXPECT_TRUE(driveInfo.BuildPartitionInfo("0:1:2:3:4:5:6;"));
   EXPECT_TRUE(driveInfo.BuildPartitionInfo("1:23MB:32MB:150GB:ext4:lvm:boot;")); // impossible flags but for test purpose only

   EXPECT_FALSE(driveInfo.BuildPartitionInfo("0:1:2:3:4:5:6:7;"));
   EXPECT_FALSE(driveInfo.BuildPartitionInfo("0:1:2:3:4:5:6:7"));
}
TEST_F(DriveInfoCommandTest, BuildPartitionInfo_TestTheFirstDevice) {
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.86.nodas.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   std::string parted = partedReading.result;
   auto devices = stringfix::explode("BYT;", parted);
   ASSERT_EQ(7, devices.size());
   ASSERT_TRUE(devices[0].empty());

   MockDriveInfo driveInfo("dummy");
   auto device0 = stringfix::split("\n", devices[1]);
   ASSERT_EQ(device0.size(), 3);

   auto partitions = driveInfo.GetPartitionInfo();
   ASSERT_EQ(partitions.size(), 0);

   ASSERT_EQ(device0[1], "1:1049kB:525MB:524MB:ext4::boot;");
   ASSERT_TRUE(driveInfo.BuildPartitionInfo(device0[1]));
   partitions = driveInfo.GetPartitionInfo();
   ASSERT_EQ(partitions.size(), 1);


   ASSERT_EQ(device0[2], "2:525MB:299GB:299GB:::lvm;");
   ASSERT_TRUE(driveInfo.BuildPartitionInfo(device0[2]));
   partitions = driveInfo.GetPartitionInfo();
   ASSERT_EQ(partitions.size(), 2);


   const auto& part1 = partitions[0];
   EXPECT_EQ(1, part1.number());
   EXPECT_EQ(1049 << KB_TO_B_SHIFT, part1.startinb());
   EXPECT_EQ(525  << MB_TO_B_SHIFT, part1.endinb());
   EXPECT_EQ(524  << MB_TO_B_SHIFT, part1.sizeinb());
   EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
   EXPECT_EQ("ext4", part1.filesystem());
   EXPECT_EQ("boot", part1.flags());

   const auto& part2 = partitions[1];
   EXPECT_EQ(2, part2.number());
   EXPECT_EQ(size_t(525) << MB_TO_B_SHIFT, part2.startinb());
   EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.endinb());
   EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.sizeinb());
   EXPECT_FALSE(part2.has_type()); // can not be extracted from machine readable
   EXPECT_EQ("", part2.filesystem()); // not given
   EXPECT_EQ("lvm", part2.flags());

}
TEST_F(DriveInfoCommandTest, BuildPartitionInfo_TestTheLastDevice) {
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.86.nodas.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   std::string parted = partedReading.result;
   auto devices = stringfix::explode("BYT;", parted);
   ASSERT_EQ(7, devices.size());
   ASSERT_TRUE(devices[0].empty());

   MockDriveInfo driveInfo("dummy");
   auto device6 = stringfix::split("\n", devices[6]);
   ASSERT_EQ(device6.size(), 2);

   ASSERT_EQ(device6[0], "/dev/mapper/vg_probe00-lv_root:211GB:dm:512:512:loop:Linux device-mapper (linear);");
   ASSERT_EQ(device6[1], "1:0.00B:211GB:211GB:ext4::;");
   ASSERT_TRUE(driveInfo.BuildPartitionInfo(device6[1]));
   auto partitions = driveInfo.GetPartitionInfo();
   ASSERT_EQ(partitions.size(), 1);

   const auto& part1 = partitions[0];
   EXPECT_EQ(1, part1.number());
   EXPECT_EQ(0, part1.startinb());
   EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.endinb());
   EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.sizeinb());
   EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
   EXPECT_EQ("ext4", part1.filesystem());
   EXPECT_EQ("", part1.flags());
}
TEST_F(DriveInfoCommandTest, ExecuteWhiteBoxCommandExtractDrivesAndPartitions) {
   MockDriveInfoCommand testCommand(cmd, autoManagedManager);
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.86.nodas.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   std::vector<DriveInfo> drives = testCommand.ExtractPartedToDrives(partedReading.result);
   ASSERT_EQ(drives.size(), 6);

   // verify the first and the last drive
   {
      // FIRST DRIVE
      const auto& drive0 = drives[0];
      EXPECT_EQ(drive0.model(), "DELL PERC H710 (scsi)");
      EXPECT_EQ(drive0.device(), "/dev/sda");
      EXPECT_EQ(drive0.capacityinb(), size_t(299) << GB_TO_B_SHIFT);
      EXPECT_EQ(drive0.table(), "msdos");

      // Check Drive0's partitions
      auto partitions = drive0.GetPartitionInfo();
      ASSERT_EQ(partitions.size(), 2);
      const auto& part1 = partitions[0];
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(size_t(1049) << KB_TO_B_SHIFT, part1.startinb());
      EXPECT_EQ(size_t(525)  << MB_TO_B_SHIFT, part1.endinb());
      EXPECT_EQ(size_t(524)  << MB_TO_B_SHIFT, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("boot", part1.flags());

      const auto& part2 = partitions[1];
      EXPECT_EQ(2, part2.number());
      EXPECT_EQ(size_t(525) << MB_TO_B_SHIFT, part2.startinb());
      EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.endinb());
      EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.sizeinb());
      EXPECT_FALSE(part2.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("", part2.filesystem()); // not given
      EXPECT_EQ("lvm", part2.flags());
   }

   { // LAST DRIVE
      const auto& drive5 = drives[5];
      EXPECT_EQ(drive5.model(), "Linux device-mapper (linear) (dm)");
      EXPECT_EQ(drive5.device(), "/dev/mapper/vg_probe00-lv_root");
      EXPECT_EQ(drive5.capacityinb(), size_t(211) << GB_TO_B_SHIFT);
      EXPECT_EQ(drive5.table(), "loop");

      // Check Drive5's partition
      auto partitions = drive5.GetPartitionInfo();
      ASSERT_EQ(partitions.size(), 1);

      const auto& part1 = partitions[0];
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(0, part1.startinb());
      EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.endinb());
      EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("", part1.flags());
   }
}
TEST_F(DriveInfoCommandTest, ExecuteWhiteBoxCommandOnNoDas86) {
   MockDriveInfoCommand testCommand(cmd, autoManagedManager);
   autoManagedManager.SetSuccess(true);
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.86.nodas.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   autoManagedManager.SetResult(partedReading.result);

   protoMsg::CommandReply reply = testCommand.Execute(conf);
   EXPECT_EQ(autoManagedManager.getRunCommand(), "/usr/local/probe/sbin/parted");
   EXPECT_EQ(autoManagedManager.getRunArgs(), " -lm print");

   EXPECT_TRUE(reply.success());
   protoMsg::DrivesInfo drives;
   ASSERT_TRUE(drives.ParseFromString(reply.result())) << "\nRESULT: " << reply.result();

   size_t numberOfDrives = drives.drives_size();
   ASSERT_EQ(numberOfDrives, 6);

   // Verify first and last
   {
      // First Drive
      const auto& drive0 = drives.drives(0);
      EXPECT_EQ(drive0.model(), "DELL PERC H710 (scsi)");
      EXPECT_EQ(drive0.device(), "/dev/sda");
      EXPECT_EQ(drive0.capacityinb(), size_t(299) << GB_TO_B_SHIFT);
      EXPECT_EQ(drive0.table(), "msdos");

      // Check Drive0's partitions
      size_t numberOfPartitions = drive0.partitions_size();
      ASSERT_EQ(numberOfPartitions, 2);
      const auto& part1 = drive0.partitions(0);
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(size_t(1049) << KB_TO_B_SHIFT , part1.startinb());
      EXPECT_EQ(size_t(525) << MB_TO_B_SHIFT, part1.endinb());
      EXPECT_EQ(size_t(524) << MB_TO_B_SHIFT, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("boot", part1.flags());


      const auto& part2 = drive0.partitions(1);
      EXPECT_EQ(2, part2.number());
      EXPECT_EQ(size_t(525) << MB_TO_B_SHIFT, part2.startinb());
      EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.endinb());
      EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.sizeinb());
      EXPECT_FALSE(part2.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("", part2.filesystem()); // not given
      EXPECT_EQ("lvm", part2.flags());
   }
   {
      // Last Drive
      const auto& drive5 = drives.drives(5);
      EXPECT_EQ(drive5.model(), "Linux device-mapper (linear) (dm)");
      EXPECT_EQ(drive5.device(), "/dev/mapper/vg_probe00-lv_root");
      EXPECT_EQ(drive5.capacityinb(), size_t(211) << GB_TO_B_SHIFT);
      EXPECT_EQ(drive5.table(), "loop");

      // Check Drive5's partition
      size_t numberOfPartitions = drive5.partitions_size();
      ASSERT_EQ(numberOfPartitions, 1);
      const auto& part1 = drive5.partitions(0);

      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(0, part1.startinb());
      EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.endinb());
      EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("", part1.flags());
   }
}


// *.119 also gives out some parted errors which we should 
// successfully handle. Ref Research/test/resources/parted.119.das.txt
TEST_F(DriveInfoCommandTest, ExecuteWhiteBoxCommandOnDas119) {
   MockDriveInfoCommand testCommand(cmd, autoManagedManager);
   autoManagedManager.SetSuccess(true);
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.119.das.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   autoManagedManager.SetResult(partedReading.result);

   protoMsg::CommandReply reply = testCommand.Execute(conf);
   EXPECT_EQ(autoManagedManager.getRunCommand(), "/usr/local/probe/sbin/parted");
   EXPECT_EQ(autoManagedManager.getRunArgs(), " -lm print");

   EXPECT_TRUE(reply.success());
   protoMsg::DrivesInfo drives;
   ASSERT_TRUE(drives.ParseFromString(reply.result())) << "\nRESULT: " << reply.result();

   size_t numberOfDrives = drives.drives_size();
   ASSERT_EQ(numberOfDrives, 7);

   // Verify ALL
   {
      // First Drive
      const auto& drive0 = drives.drives(0);
      EXPECT_EQ(drive0.model(), "DELL PERC H710 (scsi)");
      EXPECT_EQ(drive0.device(), "/dev/sda");
      EXPECT_EQ(drive0.capacityinb(), size_t(299) << GB_TO_B_SHIFT);
      EXPECT_EQ(drive0.table(), "msdos");

      // Check Drive's partitions
      size_t numberOfPartitions = drive0.partitions_size();
      ASSERT_EQ(numberOfPartitions, 2);
      const auto& part1 = drive0.partitions(0);
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(size_t(1049) << KB_TO_B_SHIFT, part1.startinb());
      EXPECT_EQ(size_t(525) << MB_TO_B_SHIFT, part1.endinb());
      EXPECT_EQ(size_t(524) << MB_TO_B_SHIFT, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("boot", part1.flags());


      const auto& part2 = drive0.partitions(1);
      EXPECT_EQ(2, part2.number());
      EXPECT_EQ(size_t(525) << MB_TO_B_SHIFT, part2.startinb());
      EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.endinb());
      EXPECT_EQ(size_t(299) << GB_TO_B_SHIFT, part2.sizeinb());
      EXPECT_FALSE(part2.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("", part2.filesystem()); // not given
      EXPECT_EQ("lvm", part2.flags());
   }

   {
      // 2nd Drive
      ///dev/sdb:1799GB:scsi:512:512:msdos:DELL PERC H710;
      //1:1049kB:1799GB:1799GB:::lvm;
      const auto& drive1 = drives.drives(1);
      EXPECT_EQ(drive1.model(), "DELL PERC H710 (scsi)");
      EXPECT_EQ(drive1.device(), "/dev/sdb");
      EXPECT_EQ(drive1.capacityinb(), size_t(1799) << GB_TO_B_SHIFT);
      EXPECT_EQ(drive1.table(), "msdos");

      // Check Drive2's partitions
      size_t numberOfPartitions = drive1.partitions_size();
      ASSERT_EQ(numberOfPartitions, 1);
      const auto& part1 = drive1.partitions(0);
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ((size_t(1049)) << KB_TO_B_SHIFT, part1.startinb());
      EXPECT_EQ((size_t(1799)) << GB_TO_B_SHIFT, part1.endinb());
      EXPECT_EQ((size_t(1799)) << GB_TO_B_SHIFT, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("", part1.filesystem());
      EXPECT_EQ("lvm", part1.flags());
   }

   {
      // 3nd Drive
      ///dev/sdb:1799GB:scsi:512:512:msdos:DELL PERC H710;
      //1:1049kB:1799GB:1799GB:::lvm;
      // 26.4 TB
      auto dasSizeInTB = size_t(26) << TB_TO_B_SHIFT;
      dasSizeInTB += size_t(400) << GB_TO_B_SHIFT;
      const auto& drive2 = drives.drives(2);
      EXPECT_EQ(drive2.model(), "DELL PERC H810 (scsi)");
      EXPECT_EQ(drive2.device(), "/dev/sdc");
      EXPECT_EQ(drive2.capacityinb(), dasSizeInTB); // 26.4TB
      EXPECT_EQ(drive2.table(), "gpt");

      // Check Drive's partitions
      size_t numberOfPartitions = drive2.partitions_size();
      ASSERT_EQ(numberOfPartitions, 1);
      const auto& part1 = drive2.partitions(0);
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ((size_t(1049)) << KB_TO_B_SHIFT, part1.startinb());
      EXPECT_EQ(dasSizeInTB, part1.endinb());
      EXPECT_EQ(dasSizeInTB, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("", part1.filesystem());
      EXPECT_EQ("lvm", part1.flags());
   }

   {
      // 4nd Drive : Has parted errors, complains about partition length
      //dev/mapper/vg_probe00-lv_home:53.7GB:dm:512:512:loop:Linux device-mapper (linear);
      //1:0.00B:53.7GB:53.7GB:ext4::;
      size_t sizeOfDrive = size_t(53) << GB_TO_B_SHIFT; // 53.7GB
      sizeOfDrive += size_t(700) << MB_TO_B_SHIFT;
      const auto& drive3 = drives.drives(3);
      EXPECT_EQ(drive3.model(), "Linux device-mapper (linear) (dm)");
      EXPECT_EQ(drive3.device(), "/dev/mapper/vg_probe00-lv_home");
      EXPECT_EQ(drive3.capacityinb(), sizeOfDrive);
      EXPECT_EQ(drive3.table(), "loop");

      // Check Drive's partitions
      size_t numberOfPartitions = drive3.partitions_size();
      ASSERT_EQ(numberOfPartitions, 1);
      const auto& part1 = drive3.partitions(0);
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(0, part1.startinb());
      EXPECT_EQ(sizeOfDrive, part1.endinb());
      EXPECT_EQ(sizeOfDrive, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("", part1.flags());
   }

   {
      // 5th Drive
      //   /dev/mapper/vg_probe01-lv_data:1799GB:dm:512:512:loop:Linux device-mapper (linear);
      //   1:0.00B:1799GB:1799GB:ext4::;
      size_t sizeOfDrive = size_t(1799) << GB_TO_B_SHIFT; // 53.7GB
      const auto& drive4 = drives.drives(4);
      EXPECT_EQ(drive4.model(), "Linux device-mapper (linear) (dm)");
      EXPECT_EQ(drive4.device(), "/dev/mapper/vg_probe01-lv_data");
      EXPECT_EQ(drive4.capacityinb(), sizeOfDrive);
      EXPECT_EQ(drive4.table(), "loop");

      // Check Drive5's partitions
      size_t numberOfPartitions = drive4.partitions_size();
      ASSERT_EQ(numberOfPartitions, 1);
      const auto& part1 = drive4.partitions(0);
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(0, part1.startinb());
      EXPECT_EQ(sizeOfDrive, part1.endinb());
      EXPECT_EQ(sizeOfDrive, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("", part1.flags());
   }

   {
      // 6th Drive
      // /dev/mapper/vg_probe00-lv_swap:33.8GB:dm:512:512:loop:Linux device-mapper (linear);
      //  1:0.00B:33.8GB:33.8GB:linux-swap(v1)::;
      size_t sizeOfDrive = size_t(33) << GB_TO_B_SHIFT; // 33.8 GB
      sizeOfDrive += size_t(800) << MB_TO_B_SHIFT;
      
      const auto& drive5 = drives.drives(5);
      EXPECT_EQ(drive5.model(), "Linux device-mapper (linear) (dm)");
      EXPECT_EQ(drive5.device(), "/dev/mapper/vg_probe00-lv_swap");
      EXPECT_EQ(drive5.capacityinb(), sizeOfDrive);
      EXPECT_EQ(drive5.table(), "loop");

      // Check partitions
      size_t numberOfPartitions = drive5.partitions_size();
      ASSERT_EQ(numberOfPartitions, 1);
      const auto& part1 = drive5.partitions(0);
      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(0, part1.startinb());
      EXPECT_EQ(sizeOfDrive, part1.endinb());
      EXPECT_EQ(sizeOfDrive, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("linux-swap(v1)", part1.filesystem());
      EXPECT_EQ("", part1.flags());
   }


   {
      // 7th and Last Drive
      const auto& drive6 = drives.drives(6);
      EXPECT_EQ(drive6.model(), "Linux device-mapper (linear) (dm)");
      EXPECT_EQ(drive6.device(), "/dev/mapper/vg_probe00-lv_root");
      EXPECT_EQ(drive6.capacityinb(), size_t(211) << GB_TO_B_SHIFT);
      EXPECT_EQ(drive6.table(), "loop");

      // Check Drive's partition
      size_t numberOfPartitions = drive6.partitions_size();
      ASSERT_EQ(numberOfPartitions, 1);
      const auto& part1 = drive6.partitions(0);

      EXPECT_EQ(1, part1.number());
      EXPECT_EQ(0, part1.startinb());
      EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.endinb());
      EXPECT_EQ(size_t(211) << GB_TO_B_SHIFT, part1.sizeinb());
      EXPECT_FALSE(part1.has_type()); // can not be extracted from machine readable
      EXPECT_EQ("ext4", part1.filesystem());
      EXPECT_EQ("", part1.flags());
   }
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
TEST_F(DriveInfoCommandTest, DISABLED_TestWithSocket) {
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
