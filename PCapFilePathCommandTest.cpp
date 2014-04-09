#include "PCapFilePathCommand.h"
#include "PCapFilePathCommandTest.h"
#include "MockPCapFilePathCommand.h"
#include "DriveInfo.h"
#include "Crowbar.h"
#include "MockConf.h"
#include "FileIO.h"
#include "Conversion.h"
#include "StringFix.h"
#include "MockDriveInfo.h"
#include "UuidHash.h"

#ifdef LR_DEBUG
TEST_F(PCapFilePathCommandTest, NonPartitionedFilePath) {
   cmd.set_stringargone("123456789012345678901234567890123456");
   MockPCapFilePathCommand testCommand(cmd, autoManagedManager);
   autoManagedManager.SetSuccess(true);
   autoManagedManager.SetResult("test result");

   MockConf conf;
   conf.mOverrideGetFirstCaptureLocation = true;
   conf.mPCapCaptureLocations.push_back("/usr/local/probe/pcap");
   protoMsg::CommandRequest request;
   request.set_type(::protoMsg::CommandRequest_CommandType_PCAP_FILE_PATH);
   protoMsg::CommandReply reply = testCommand.Execute(conf);

   ASSERT_TRUE(reply.success());
   EXPECT_TRUE(reply.result().find("/usr/local/probe/pcap")!=std::string::npos) << "Cannot find pcap dir " << reply.result();
   EXPECT_TRUE(reply.result().find(cmd.stringargone())!=std::string::npos) << "Cannot find uuid filename " << reply.result();

}
TEST_F(PCapFilePathCommandTest, PartitionedFilePath) {

   
   std::string uuid1 = "553c367a-f638-457c-9916-624e189702ef";
   cmd.set_stringargone(uuid1);
   MockPCapFilePathCommand testCommand(cmd, autoManagedManager);
   autoManagedManager.SetSuccess(true);
   autoManagedManager.SetResult("test result");

   MockConf conf;
   conf.mOverrideGetFirstCaptureLocation = true;
   conf.mPCapCaptureLocations.push_back("/usr/local/probe/pcap0");
   conf.mPCapCaptureLocations.push_back("/usr/local/probe/pcap1");
   protoMsg::CommandRequest request;
   request.set_type(::protoMsg::CommandRequest_CommandType_PCAP_FILE_PATH);
   protoMsg::CommandReply reply = testCommand.Execute(conf);

   size_t bucket1 = UuidHash::GetUuidBucket(uuid1, conf.GetPcapCaptureFolderPerPartitionLimit(), conf.mPCapCaptureLocations.size());
   size_t bucketDir1 = UuidHash::GetUuidBucketDirectoryIndex(bucket1, conf.GetPcapCaptureFolderPerPartitionLimit());

   ASSERT_TRUE(reply.success());
   std::string expected1 = {"/usr/local/probe/pcap0/" + std::to_string(bucketDir1)};
   EXPECT_TRUE(reply.result().find(expected1)!=std::string::npos) << "Cannot find pcap dir " << reply.result() << ". expected1: " << expected1;
   EXPECT_TRUE(reply.result().find(cmd.stringargone())!=std::string::npos) << "Cannot find uuid filename " << reply.result();
   
   std::string uuid2 = "123456789012345678901234567890123401";
   size_t bucket2 = UuidHash::GetUuidBucket(uuid2, conf.GetPcapCaptureFolderPerPartitionLimit(), conf.mPCapCaptureLocations.size());
   size_t bucketDir2 = UuidHash::GetUuidBucketDirectoryIndex(bucket2, conf.GetPcapCaptureFolderPerPartitionLimit());
   cmd.set_stringargone(uuid2);
   MockPCapFilePathCommand testCommand2(cmd, autoManagedManager);
   reply = testCommand2.Execute(conf);
   ASSERT_TRUE(reply.success());
   std::string expected2 = {"/usr/local/probe/pcap0/" + std::to_string(bucketDir2)};
   EXPECT_TRUE(reply.result().find(expected2)!=std::string::npos) << "Cannot find pcap dir " << reply.result() << ". expected2: " << expected2;
   EXPECT_TRUE(reply.result().find(cmd.stringargone())!=std::string::npos) << "Cannot find uuid filename " << reply.result();

}
TEST_F(PCapFilePathCommandTest, EmptyPartitionedFilePath) {
   cmd.set_stringargone("123456789012345678901234567890123456");
   MockPCapFilePathCommand testCommand(cmd, autoManagedManager);
   autoManagedManager.SetSuccess(true);
   autoManagedManager.SetResult("test result");

   MockConf conf;
   conf.mOverrideGetFirstCaptureLocation = true;
   conf.mPCapCaptureLocations.push_back("");
   protoMsg::CommandRequest request;
   request.set_type(::protoMsg::CommandRequest_CommandType_PCAP_FILE_PATH);
   protoMsg::CommandReply reply = testCommand.Execute(conf);

   ASSERT_FALSE(reply.success()) << ": result: " << reply.result();
   EXPECT_TRUE(reply.result().empty());

}
TEST_F(PCapFilePathCommandTest, InvalidUUID) {
   cmd.set_stringargone("");
   MockPCapFilePathCommand testCommand(cmd, autoManagedManager);
   autoManagedManager.SetSuccess(true);
   autoManagedManager.SetResult("test result");

   MockConf conf;
   conf.mOverrideGetFirstCaptureLocation = true;
   conf.mPCapCaptureLocations.push_back("/usr/local/probe/pcap/0/");
   protoMsg::CommandRequest request;
   request.set_type(::protoMsg::CommandRequest_CommandType_PCAP_FILE_PATH);
   protoMsg::CommandReply reply = testCommand.Execute(conf);

   ASSERT_FALSE(reply.success());
   EXPECT_TRUE(reply.result().empty());

}
#else 
TEST_F(PCapFilePathCommandTest, NullTest) {
   EXPECT_TRUE(true);
}

#endif
