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

TEST_F(RESTParserTest, GetIdsFromSearchForRecordsWithStaleInfo) {
   RESTParser parser;
   
   std::string resultString = "{\"took\":113,\"timed_out\":false,\"_shards\":"
           "{\"total\":69,\"successful\":66,\"failed\":3,\"failures\":"
           "[{\"index\":\"kibana-int\",\"shard\":1,\"status\":400,\"reason\":"
           "\"foo\"},";
 //          "\"SearchParseException[[kibana-int][1]: from[-1],size[-1]: Parse Failure [Failed to parse source [\n{\"sort\": [ { \"time_updated\": { \"order\" : \"asc\", \"ignore_unmapped\" : true } } ],\"query\" : {\"filtered\" :{\"filter\" : {\"bool\" :{\"must\": [{ \"term\" : {\"captureRemoved\" : true}},{ \"term\" : {\"latest_update\" : true}},{ \"numeric_range\" : {\"flowSessionCount\" : { \"gt\" : \"1\" }}}]}}},\"_cache\":false,\"from\": 0,\"size\":1,\"fields\": [\"session_id\", \"time_updated\", \"time_start\"]}}]]]; nested: QueryParsingException[[kibana-int] failed to find mapping for field [flowSessionCount]]; \"},"
   resultString+="{\"index\":\"kibana-int\",\"shard\":0,\"status\":400,\"reason\":"
            "\"foo\"},";
//          "\"SearchParseException[[kibana-int][0]: from[-1],size[-1]: Parse Failure [Failed to parse source [\n{\"sort\": [ { \"time_updated\": { \"order\" : \"asc\", \"ignore_unmapped\" : true } } ],\"query\" : {\"filtered\" :{\"filter\" : {\"bool\" :{\"must\": [{ \"term\" : {\"captureRemoved\" : true}},{ \"term\" : {\"latest_update\" : true}},{ \"numeric_range\" : {\"flowSessionCount\" : { \"gt\" : \"1\" }}}]}}},\"_cache\":false,\"from\": 0,\"size\":1,\"fields\": [\"session_id\", \"time_updated\", \"time_start\"]}}]]]; nested: QueryParsingException[[kibana-int] failed to find mapping for field [flowSessionCount]]; \"},"
   resultString+="{\"index\":\"upgrade\",\"shard\":0,\"status\":400,\"reason\":"
            "\"foo\"}]},";
//          "\"SearchParseException[[upgrade][0]: from[-1],size[-1]: Parse Failure [Failed to parse source [\n{\"sort\": [ { \"time_updated\": { \"order\" : \"asc\", \"ignore_unmapped\" : true } } ],\"query\" : {\"filtered\" :{\"filter\" : {\"bool\" :{\"must\": [{ \"term\" : {\"captureRemoved\" : true}},{ \"term\" : {\"latest_update\" : true}},{ \"numeric_range\" : {\"flowSessionCount\" : { \"gt\" : \"1\" }}}]}}},\"_cache\":false,\"from\": 0,\"size\":1,\"fields\": [\"session_id\", \"time_updated\", \"time_start\"]}}]]]; nested: QueryParsingException[[upgrade] failed to find mapping for field [flowSessionCount]]; \"}]},"
   resultString+="\"hits\":{\"total\":279,\"max_score\":null,\"hits\":"
           "[{\"_index\":\"network_2013_11_11\",\"_type\":\"meta\",\"_id\":\"4fc48be8-b1ee-4b58-b9d6-4f58722ec6d8_14\","
           "\"_score\":null,\"fields\":{\"session_id\":\"4fc48be8-b1ee-4b58-b9d6-4f58722ec6d8\",\"time_updated\":\"2013/11/11 16:56:33\","
           "\"time_start\":\"2013/11/11 14:31:01\"},\"sort\":[1384188993000]}]}}";
   
   auto oldestSessionIds = parser.GetSessionIds(resultString);
   ASSERT_FALSE(oldestSessionIds.empty());
   EXPECT_TRUE("4fc48be8-b1ee-4b58-b9d6-4f58722ec6d8"==*oldestSessionIds.begin());
   
}
TEST_F(RESTParserTest, GetOldesttime_start) {
   RESTParser parser;
   
   std::string resultString = 
           "200|ok|"
           "{\"took\":55,\"timed_out\":false,\"_shards\":"
              "{\"total\":2,\"successful\":2,\"failed\":0},"
              "\"hits\":{\"total\":5653,\"max_score\":null,\"hits\":";
   resultString += 
              "["
                 "{\"_index\":\"network_2013_11_05\",\"_type\":\"meta\",\"_id\":"
                 "\"57fe41e4-c34d-47a8-90bb-1c77f1f15ffa_8\",\"_score\":null,\"fields\":"
                    "{\"session_id\":\"57fe41e4-c34d-47a8-90bb-1c77f1f15ffa\",\"time_updated\":"
                    "\"2013/11/05 20:06:37\",\"time_start\":\"2009/02/13 23:31:30\"},"
                 "\"sort\":[1383681997000]}";
   resultString += 
                 ",{\"_index\":\"network_2013_11_05\",\"_type\":\"meta\",\"_id\":"
                 "\"f922b594-df00-4470-ab6b-0add450af6a0_8\",\"_score\":null,\"fields\":"
                    "{\"session_id\":\"f922b594-df00-4470-ab6b-0add450af6a0\",\"time_updated\":"
                    "\"2013/11/05 20:06:37\",\"time_start\":\"2009/02/13 23:31:31\"},"
                 "\"sort\":[1383681997000]}";
   resultString += 
              "]"
           "}"
           "}";
   EXPECT_EQ(1234567890L,parser.GetOldestTimeField(resultString,"time_start"));
   
                      
   
}
TEST_F(RESTParserTest, ParseESTimestamp) {
   RESTParser parser;
   
   EXPECT_EQ(0, parser.ParseESTimestamp("no/timestamp/here you:foo:!"));
   EXPECT_EQ(0, parser.ParseESTimestamp("1970/01/01 00:00:00"));
   EXPECT_EQ(0, parser.ParseESTimestamp("1000/01/01 00:00:00"));
   EXPECT_EQ(1234567890L, parser.ParseESTimestamp("2009/02/13 23:31:30"));
   
}

TEST_F(RESTParserTest, GetOldestTimeField) {
   RESTParser parser;
   std::string testReply = "200|ok|{\"took\":4359,\"timed_out\":false,\"_shards\":{\"total\":84,\"successful\":84,\"failed\":0},\"hits\":{\"total\":13992297,\"atotal\":123,\"totalb\":345,\"hits\":[{\"_index\":\"network_2013_10_28\",\"_type\":\"meta\",\"_id\":\"226890cf-a90a-42ea-b7d9-5e3492b488a6\",\"_score\":null,\"fields\":{\"time_updated\":\"2009/02/13 23:31:30\",\"timeAgain\":\"2009/02/13 23:31:31\"},\"sort\":[1382976468000]}]}}";
   EXPECT_EQ(1234567890L,parser.GetOldestTimeField(testReply,"time_updated"));
   EXPECT_EQ(1234567891L,parser.GetOldestTimeField(testReply,"timeAgain"));
   EXPECT_EQ(0,parser.GetOldestTimeField(testReply,"notHere"));
   testReply = "200|ok|{\"took\":1759,\"timed_out\":false,\"_shards\":{\"total\":1,\"successful\":1,"
           "\"failed\":0},\"hits\":{\"total\":2,\"max_score\":null,"
           "\"hits\":[{\"_index\":\"upgrade\",\"_type\":\"info\",\"_id\":\"1383325709\","
           "\"_score\":null, \"_source\" : {\"upgradeDate\":\"2009/02/13 23:31:30\","
           "\"ignorePreviousData\":true,\"upgradingToVersion\":\"1235\"},\"sort\":[1383325709000]}]}}";
   EXPECT_EQ(1234567890L,parser.GetOldestTimeField(testReply,"upgradeDate"));
   testReply = "200|ok|{\"_shards\":{\"total\":84,\"successful\":84,\"failed\":0},\"hits\":{\"total\":13992297,\"atotal\":123,\"totalb\":345}}";
   EXPECT_EQ(0,parser.GetOldestTimeField(testReply,"time_updated"));
   EXPECT_EQ(0,parser.GetOldestTimeField(testReply,"timeAgain"));
   EXPECT_EQ(0,parser.GetOldestTimeField(testReply,"notHere"));
   testReply = "";
   EXPECT_EQ(0,parser.GetOldestTimeField(testReply,"time_updated"));
}

TEST_F(RESTParserTest, GetTotalHits) {
   RESTParser sender;

   std::string fakeReply;

   EXPECT_EQ(0, sender.GetTotalHits(fakeReply));
   fakeReply = "200|ok|{\"here\" : {\"us chickens\", \"nobody\" } }";
   EXPECT_EQ(0, sender.GetTotalHits(fakeReply));
   fakeReply = "200|ok|{\"took\":4359,\"timed_out\":false,\"_shards\":{\"total\":84,\"successful\":84,\"failed\":0},\"hits\":{\"total\":13992297,\"max_score\":null,\"hits\":[{\"_index\":\"network_2013_10_28\",\"_type\":\"meta\",\"_id\":\"226890cf-a90a-42ea-b7d9-5e3492b488a6\",\"_score\":null,\"fields\":{\"time_updated\":\"2013/10/28 16:07:48\",\"session_id\":\"226890cf-a90a-42ea-b7d9-5e3492b488a6\"},\"sort\":[1382976468000]}]}}";
   EXPECT_EQ(13992297, sender.GetTotalHits(fakeReply));
   fakeReply = "200|ok|{\"took\":4359,\"timed_out\":false,\"_shards\":{\"total\":84,\"successful\":84,\"failed\":0},\"hits\":{\"total\":13992297,\"atotal\":123,\"totalb\":345,\"hits\":[{\"_index\":\"network_2013_10_28\",\"_type\":\"meta\",\"_id\":\"226890cf-a90a-42ea-b7d9-5e3492b488a6\",\"_score\":null,\"fields\":{\"time_updated\":\"2013/10/28 16:07:48\",\"session_id\":\"226890cf-a90a-42ea-b7d9-5e3492b488a6\"},\"sort\":[1382976468000]}]}}";
   EXPECT_EQ(13992297, sender.GetTotalHits(fakeReply));
}

TEST_F(RESTParserTest, GetCount) {

   RESTParser sender;

   std::string fakeReply;

   EXPECT_EQ(0, sender.GetCount(fakeReply));
   fakeReply = "200|ok|{\"here\" : {\"us chickens\", \"nobody\" } }";
   EXPECT_EQ(0, sender.GetCount(fakeReply));
   fakeReply = "200|ok|{\"count\":12147998,\"_shards\":{\"total\":78,\"successful\":78,\"failed\":0}}";
   EXPECT_EQ(12147998, sender.GetCount(fakeReply));
   fakeReply = "200|ok|{\"count\":12147998,\"counterattack\":123,\"kitchencounter\":456}";
   EXPECT_EQ(12147998, sender.GetCount(fakeReply));
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
   ASSERT_TRUE(sender.GetAClustersDiskInfo("b", info, retrievedInfo));
   EXPECT_EQ(singleInfo, retrievedInfo);
   std::get<0>(singleInfo)["a"] = "A";
   std::get<1>(singleInfo)["nA"] = 1;
   ASSERT_TRUE(sender.GetAClustersDiskInfo("a", info, retrievedInfo));
   EXPECT_EQ(singleInfo, retrievedInfo);
   EXPECT_FALSE(sender.GetAClustersDiskInfo("c", info, retrievedInfo));

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

   EXPECT_EQ(10, ids.size());

   EXPECT_TRUE(StringContains(ids[0], "2e0bb480-be41-427c-92b5-61ccedbe5d6a"));
   EXPECT_TRUE(StringContains(ids[1], "989a6965-a19a-44b4-9f75-cf283fc44b30"));
   EXPECT_TRUE(StringContains(ids[2], "6ca68984-3333-431f-8a29-763524ad4f3f"));
   EXPECT_TRUE(StringContains(ids[3], "de2bd6fe-95dc-4058-9b5c-dd0f6821add7"));
   EXPECT_TRUE(StringContains(ids[4], "64fe3bb6-312d-4353-b9e0-abbc61303b99"));
   EXPECT_TRUE(StringContains(ids[5], "0a368a48-d4ad-4edf-8aff-e6b49da4d38b"));
   EXPECT_TRUE(StringContains(ids[6], "c79802e0-1021-43f4-af16-7be26db0363a"));
   EXPECT_TRUE(StringContains(ids[7], "ec4b8906-a3e8-4024-97c6-8617848339db"));
   EXPECT_TRUE(StringContains(ids[8], "648e4bfc-6198-4170-abb9-840e761381cf"));
   EXPECT_TRUE(StringContains(ids[9], "99d603f6-25e9-4d40-8808-364f4cbcb229"));




}

#else

TEST_F(RESTParserTest, emptyTest) {
   EXPECT_TRUE(true);
}
#endif