#include "RESTParserTest.h"
#include "MockBoomStick.h"
#include <algorithm>
#include "FileIO.h"
namespace {
   bool StringContains(const std::string& input, const std::string& pattern) {
      if (input.find(pattern) != std::string::npos) {
         return true;
      } else {
         std::cout << input << " does not contain " << pattern << std::endl;
      }
      return false;
   }
}
#ifdef LR_DEBUG

TEST_F(RESTParserTest, GetCount) {
 
   RESTParser sender;
   
   std::string fakeReply;
   
   EXPECT_EQ(0,sender.GetCount(fakeReply));
   fakeReply = "200|ok|{\"here\" : {\"us chickens\", \"nobody\" } }";
   EXPECT_EQ(0,sender.GetCount(fakeReply));
   fakeReply = "200|ok|{\"count\":12147998,\"_shards\":{\"total\":78,\"successful\":78,\"failed\":0}}";
   EXPECT_EQ(12147998,sender.GetCount(fakeReply));
   fakeReply = "200|ok|{\"count\":12147998,\"counterattack\":123,\"kitchencounter\":456}";
   EXPECT_EQ(12147998,sender.GetCount(fakeReply));
}

TEST_F(RESTParserTest, GetListOfClusterNames) {
   DiskInformation info;
   SingleDiskInfo singleInfo;
   info["a"] = singleInfo;
   info["b"] = singleInfo;
   info["c"] = singleInfo;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender;

   std::vector<std::string> names = sender.GetAllClusterNamesFromDiskInfo(info);

   ASSERT_EQ(3, names.size());
   std::string findme("a");
   EXPECT_TRUE(std::find(names.begin(), names.end(), findme) != names.end());
   findme = "b";
   EXPECT_TRUE(std::find(names.begin(), names.end(), findme) != names.end());
   findme = "c";
   EXPECT_TRUE(std::find(names.begin(), names.end(), findme) != names.end());
}

TEST_F(RESTParserTest, BadReqeust) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender;
   std::string reply = "400|BAD_REQUEST|{\"error\":\"SearchPhaseExecutionException\"}";
   EXPECT_TRUE(sender.BadReqeust(reply));
   reply = "200|OK|{}";
   EXPECT_FALSE(sender.BadReqeust(reply));
   
}

TEST_F(RESTParserTest, GetSingleDiskInfo) {
   DiskInformation info;
   SingleDiskInfo singleInfo;
   std::get<0>(singleInfo)["a"] = "A";
   std::get<1>(singleInfo)["nA"] = 1;
   info["a"] = singleInfo;
   std::get<0>(singleInfo)["a"] = "B";
   std::get<1>(singleInfo)["nA"] = 2;
   info["b"] = singleInfo;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender;

   SingleDiskInfo retrievedInfo;
   ASSERT_TRUE(sender.GetAClustersDiskInfo("b", info,retrievedInfo));
   EXPECT_EQ(singleInfo, retrievedInfo);
   std::get<0>(singleInfo)["a"] = "A";
   std::get<1>(singleInfo)["nA"] = 1;
   ASSERT_TRUE(sender.GetAClustersDiskInfo("a", info,retrievedInfo));
   EXPECT_EQ(singleInfo, retrievedInfo);
   EXPECT_FALSE(sender.GetAClustersDiskInfo("c",info,retrievedInfo));

}

TEST_F(RESTParserTest, GetSpecificDiskInfo) {
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
   RESTParser sender;
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

TEST_F(RESTParserTest, ParseForSessionIds) {
   using namespace FileIO;
   
   auto results = ReadAsciiFileContent("resources/filteredQueryTest");
   ASSERT_FALSE(results.HasFailed());
   auto reply = results.result;
   
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender;
   
   auto ids = sender.GetSessionIdsFromQuery(reply);
   
   EXPECT_EQ(10,ids.size());

   EXPECT_TRUE(StringContains(ids[0],"2e0bb480-be41-427c-92b5-61ccedbe5d6a"));
   EXPECT_TRUE(StringContains(ids[1],"989a6965-a19a-44b4-9f75-cf283fc44b30"));
   EXPECT_TRUE(StringContains(ids[2],"6ca68984-3333-431f-8a29-763524ad4f3f"));
   EXPECT_TRUE(StringContains(ids[3],"de2bd6fe-95dc-4058-9b5c-dd0f6821add7"));
   EXPECT_TRUE(StringContains(ids[4],"64fe3bb6-312d-4353-b9e0-abbc61303b99"));
   EXPECT_TRUE(StringContains(ids[5],"0a368a48-d4ad-4edf-8aff-e6b49da4d38b"));
   EXPECT_TRUE(StringContains(ids[6],"c79802e0-1021-43f4-af16-7be26db0363a"));
   EXPECT_TRUE(StringContains(ids[7],"ec4b8906-a3e8-4024-97c6-8617848339db"));
   EXPECT_TRUE(StringContains(ids[8],"648e4bfc-6198-4170-abb9-840e761381cf"));
   EXPECT_TRUE(StringContains(ids[9],"99d603f6-25e9-4d40-8808-364f4cbcb229"));
   
   

   
}

#else
TEST_F(RESTParserTest, emptyTest) {
   EXPECT_TRUE(true);
}
#endif