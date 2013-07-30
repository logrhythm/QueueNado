#include "RESTSenderTest.h"
#include "MockBoomStick.h"
#include <algorithm>

TEST_F(RESTSenderTest, GetListOfClusterNames) {
   DiskInformation info;
   SingleDiskInfo singleInfo;
   info["a"] = singleInfo;
   info["b"] = singleInfo;
   info["c"] = singleInfo;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTSender sender(transport);

   std::vector<std::string> names = sender.GetAllClusterNamesFromDiskInfo(info);

   ASSERT_EQ(3, names.size());
   std::string findme("a");
   EXPECT_TRUE(std::find(names.begin(), names.end(), findme) != names.end());
   findme = "b";
   EXPECT_TRUE(std::find(names.begin(), names.end(), findme) != names.end());
   findme = "c";
   EXPECT_TRUE(std::find(names.begin(), names.end(), findme) != names.end());
}

TEST_F(RESTSenderTest, GetSingleDiskInfo) {
   DiskInformation info;
   SingleDiskInfo singleInfo;
   std::get<0>(singleInfo)["a"] = "A";
   std::get<1>(singleInfo)["nA"] = 1;
   info["a"] = singleInfo;
   std::get<0>(singleInfo)["a"] = "B";
   std::get<1>(singleInfo)["nA"] = 2;
   info["b"] = singleInfo;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTSender sender(transport);

   SingleDiskInfo retrievedInfo;
   ASSERT_TRUE(sender.GetAClustersDiskInfo("b", info,retrievedInfo));
   EXPECT_EQ(singleInfo, retrievedInfo);
   std::get<0>(singleInfo)["a"] = "A";
   std::get<1>(singleInfo)["nA"] = 1;
   ASSERT_TRUE(sender.GetAClustersDiskInfo("a", info,retrievedInfo));
   EXPECT_EQ(singleInfo, retrievedInfo);
   EXPECT_FALSE(sender.GetAClustersDiskInfo("c",info,retrievedInfo));

}

TEST_F(RESTSenderTest, GetSpecificDiskInfo) {
   DiskInformation info;
   SingleDiskInfo singleInfo;
   std::get<0>(singleInfo)["disk_write_size"] = "589.8gb";
   std::get<0>(singleInfo)["disk_read_size"] = "106.1gb";
   std::get<0>(singleInfo)["free"] = "21.9gb";
   std::get<0>(singleInfo)["dev"] = "/dev/mapper/vg_robert2-lv_home";
   std::get<0>(singleInfo)["available"] = "18.3gb";
   std::get<0>(singleInfo)["disk_queue"] = "0.2";
   std::get<0>(singleInfo)["total"] = "72.6gb";
   std::get<0>(singleInfo)["mount"] = "/home";
   std::get<0>(singleInfo)["path"] = "/usr/local/probe/db/elasticsearch/data/logrhythm/nodes/0";
   std::get<1>(singleInfo)["disk_reads"] = 2533055;
   std::get<1>(singleInfo)["available_in_bytes"] = 19653607424;
   std::get<1>(singleInfo)["disk_read_size_in_bytes"] = 113932321792;
   std::get<1>(singleInfo)["disk_write_size_in_bytes"] = 633309597696;
   std::get<1>(singleInfo)["disk_writes"] = 155087317;
   std::get<1>(singleInfo)["free_in_bytes"] = 23618060288;
   std::get<1>(singleInfo)["total_in_bytes"] = 78044545024;
   std::string clusterName = "a";
   info[clusterName] = singleInfo;

   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTSender sender(transport);
   EXPECT_EQ("589.8gb", sender.GetDiskWriteSize(clusterName, info));
   EXPECT_EQ("106.1gb", sender.GetDiskReadSize(clusterName, info));
   EXPECT_EQ("/dev/mapper/vg_robert2-lv_home", sender.GetDiskDevice(clusterName, info));
   EXPECT_EQ("21.9gb", sender.GetDiskFree(clusterName, info));
   EXPECT_EQ("18.3gb", sender.GetDiskAvailable(clusterName, info));
   EXPECT_EQ("0.2", sender.GetDiskQueue(clusterName, info));
   EXPECT_EQ("72.6gb", sender.GetDiskTotal(clusterName, info));
   EXPECT_EQ("/home", sender.GetDiskMount(clusterName, info));
   EXPECT_EQ("/usr/local/probe/db/elasticsearch/data/logrhythm/nodes/0", sender.GetDiskPath(clusterName, info));
   EXPECT_EQ(2533055, sender.GetDiskReads(clusterName, info));
   EXPECT_EQ(19653607424, sender.GetDiskAvailableInBytes(clusterName, info));
   EXPECT_EQ(113932321792, sender.GetDiskReadSizeInBytes(clusterName, info));
   EXPECT_EQ(633309597696, sender.GetDiskWriteSizeInBytes(clusterName, info));
   EXPECT_EQ(155087317, sender.GetDiskWrites(clusterName, info));
   EXPECT_EQ(23618060288, sender.GetDiskFreeInBytes(clusterName, info));
   EXPECT_EQ(78044545024, sender.GetDiskTotalInBytes(clusterName, info));
   clusterName = "bar";
   EXPECT_EQ("", sender.GetDiskWriteSize(clusterName, info));
   EXPECT_EQ("", sender.GetDiskReadSize(clusterName, info));
   EXPECT_EQ("", sender.GetDiskDevice(clusterName, info));
   EXPECT_EQ("", sender.GetDiskFree(clusterName, info));
   EXPECT_EQ("", sender.GetDiskAvailable(clusterName, info));
   EXPECT_EQ("", sender.GetDiskQueue(clusterName, info));
   EXPECT_EQ("", sender.GetDiskTotal(clusterName, info));
   EXPECT_EQ("", sender.GetDiskMount(clusterName, info));
   EXPECT_EQ("", sender.GetDiskPath(clusterName, info));
   EXPECT_EQ(0, sender.GetDiskReads(clusterName, info));
   EXPECT_EQ(0, sender.GetDiskAvailableInBytes(clusterName, info));
   EXPECT_EQ(0, sender.GetDiskReadSizeInBytes(clusterName, info));
   EXPECT_EQ(0, sender.GetDiskWriteSizeInBytes(clusterName, info));
   EXPECT_EQ(0, sender.GetDiskWrites(clusterName, info));
   EXPECT_EQ(0, sender.GetDiskFreeInBytes(clusterName, info));
   EXPECT_EQ(0, sender.GetDiskTotalInBytes(clusterName, info));
}

