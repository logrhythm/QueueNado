#include "RESTBuilderTest.h"
#include "MockBoomStick.h"
#include "ElasticSearch.h"
#include "RESTParser.h"
#include <fstream>
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

TEST_F(RESTBuilderTest, GetOldestNDocuments) {
   RESTBuilder builder; 
   
   //GetOldestNDocuments(const std::string& queryString, const unsigned int number, const bool cache,
   //                                const time_t& after=0)
   
   std::string queryString = "testQuery";
   unsigned int number(1234);
   bool cache(false);
   time_t after(1234567890L);
   
   std::string query = builder.GetOldestNDocuments(queryString,number,cache);
   std::string expectedResult = "GET|/_all/meta/_search|";
   expectedResult += "{"
           "\"sort\": ["
           "      {"
           "      \"timeUpdated\": {"
           "         \"order\" : \"asc\","
           "         \"ignore_unmapped\" : true"
           "     }"
           "   }"
           "],"
           "\"query\" :"
           "{";
   expectedResult += "   \"query_string\": {\"query\":\"";
   expectedResult += "testQuery";
   expectedResult += "\"},"
           "  \"_cache\":";
   expectedResult += "false";
   expectedResult += ","
           "\"fields\": [\"sessionId\", \"timeUpdated\"],"
           "\"from\": 0,"
           "\"size\":";
   expectedResult += "1234";
   expectedResult += "}}";
   
   EXPECT_EQ(expectedResult, query);
   query = builder.GetOldestNDocuments(queryString,number,cache,0);
   EXPECT_EQ(expectedResult, query);
   
   query = builder.GetOldestNDocuments(queryString,number,cache,after);
   expectedResult = "GET|/_all/meta/_search|";
   expectedResult += "{"
           "\"sort\": ["
           "      {"
           "      \"timeUpdated\": {"
           "         \"order\" : \"asc\","
           "         \"ignore_unmapped\" : true"
           "     }"
           "   }"
           "],"
           "\"query\" :"
           "{";
   expectedResult += "\"range\" : { \"timeUpdated\" : { \"from\" : \"";
   expectedResult += "1234567890";
   expectedResult += "\"} }";
   expectedResult += "   \"query_string\": {\"query\":\"";
   expectedResult += "testQuery";
   expectedResult += "\"},"
           "  \"_cache\":";
   expectedResult += "false";
   expectedResult += ","
           "\"fields\": [\"sessionId\", \"timeUpdated\"],"
           "\"from\": 0,"
           "\"size\":";
   expectedResult += "1234";
   expectedResult += "}}";
   EXPECT_EQ(expectedResult, query);
   
   query = builder.GetOldestNDocuments(queryString,number,true,after);
   expectedResult = "GET|/_all/meta/_search|";
   expectedResult += "{"
           "\"sort\": ["
           "      {"
           "      \"timeUpdated\": {"
           "         \"order\" : \"asc\","
           "         \"ignore_unmapped\" : true"
           "     }"
           "   }"
           "],"
           "\"query\" :"
           "{";
   expectedResult += "\"range\" : { \"timeUpdated\" : { \"from\" : \"";
   expectedResult += "1234567890";
   expectedResult += "\"} }";
   expectedResult += "   \"query_string\": {\"query\":\"";
   expectedResult += "testQuery";
   expectedResult += "\"},"
           "  \"_cache\":";
   expectedResult += "true";
   expectedResult += ","
           "\"fields\": [\"sessionId\", \"timeUpdated\"],"
           "\"from\": 0,"
           "\"size\":";
   expectedResult += "1234";
   expectedResult += "}}";
   EXPECT_EQ(expectedResult, query);
}

TEST_F(RESTBuilderTest, GetLatestUpgradeDateWhereIgnored) {
   RESTBuilder builder;
   
   std::string query = builder.GetLatestUpgradeDateWhereIgnored();
   std::string expectedReply = "GET|/upgrade/info/_search|{"
           "\"sort\" : [ { \"upgradeDate\" : { \"order\" : \"desc\" } } ],"
           "\"query\" : {"
           "\"query_string\" : { \"query\" : \"ignorePreviousDat:true\" }"
           ","
           "\"fields\": [\"upgradeDate\"],"
           "\"size\" : 1"
           "}"
           "}";
   EXPECT_TRUE(expectedReply==query);
}
TEST_F(RESTBuilderTest, GetCountQuery) {
   RESTBuilder builder;
   
   std::string query = builder.GetCountQuery("index", "type", "querystring");
   
   EXPECT_TRUE("GET|/index/type/_count|{ \"query_string\" : { \"query\" : \"querystring\" } }"==query);
}

TEST_F(RESTBuilderTest, GetIndexOptimize) {
 
   RESTBuilder builder;
   
   std::string index("thisIndex");
   unsigned int segments(123);
   bool onlyExpungeDeletes(false);
   bool flushAfter(true);
   bool waitForFinish(true);
   
   std::string command = builder.GetIndexOptimize(index,segments,onlyExpungeDeletes,flushAfter,waitForFinish);
   
   EXPECT_EQ("POST|/thisIndex/_optimize?max_num_segments=123&only_expunge_deletes=false&flush=true&wait_for_merge=true",command);

   onlyExpungeDeletes=true;
   command = builder.GetIndexOptimize(index,segments,onlyExpungeDeletes,flushAfter,waitForFinish);
   
   EXPECT_EQ("POST|/thisIndex/_optimize?max_num_segments=123&only_expunge_deletes=true&flush=true&wait_for_merge=true",command);
   flushAfter = false;
   command = builder.GetIndexOptimize(index,segments,onlyExpungeDeletes,flushAfter,waitForFinish);
   
   EXPECT_EQ("POST|/thisIndex/_optimize?max_num_segments=123&only_expunge_deletes=true&flush=false&wait_for_merge=true",command);
   waitForFinish = false;
   command = builder.GetIndexOptimize(index,segments,onlyExpungeDeletes,flushAfter,waitForFinish);
   
   EXPECT_EQ("POST|/thisIndex/_optimize?max_num_segments=123&only_expunge_deletes=true&flush=false&wait_for_merge=false",command);

   command = builder.GetIndexOptimize(index);
   EXPECT_EQ("POST|/thisIndex/_optimize?max_num_segments=1&only_expunge_deletes=false&flush=true&wait_for_merge=true",command);

   
}
TEST_F(RESTBuilderTest, GetFilteredQuerySortedByTime) {
   RESTBuilder builder;
   
   std::string command = builder.GetOldestNDocuments("captured:true",100,true);
   
   EXPECT_TRUE(StringContains(command, "\"query\":\"captured:true\""));
   
   EXPECT_TRUE(StringContains(command, "\"timeUpdated\": "));
   EXPECT_TRUE(StringContains(command, "\"order\" : \"asc\""));
   EXPECT_TRUE(StringContains(command, "\"size\":100"));
   EXPECT_TRUE(StringContains(command, "\"_cache\":true"));
   EXPECT_TRUE(StringContains(command, "GET|/_all/meta/_search|"));
   
   
}

TEST_F(RESTBuilderTest, ConstructAQuery) {
   RESTBuilder builder;

   std::string command = builder.RunQueryOnAllIndicies("indexType", "foo:bar", CACHE);

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar", command);

   command = builder.RunQueryOnAllIndicies("indexType", "foo:bar", NO_CACHE);

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar&cache=false", command);
}

TEST_F(RESTBuilderTest, ConstructAIdQuery) {
   RESTBuilder builder;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender;

   std::string command = builder.RunQueryOnlyForDocIds("indexType", "foo:bar");

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar&cache=false|{ \"fields\" : [], \"size\" : 40}", command);

   transport.mReturnString = "200|ok|{\"took\":10,\"timed_out\":false,\"_shards\":{\"total\":50,"
           "\"successful\":50,\"failed\":0},\"hits\":{\"total\":4,\"max_score\":12.653517,"
           "\"hits\":[{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_2\",\"_score\":12.653517},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_3\",\"_score\":12.650981},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_4\",\"_score\":12.650732},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_1\",\"_score\":12.649386}]}}";
   std::string reply;
   reply = transport.Send(command);
   std::vector<std::pair<std::string, std::string> > ids = sender.GetDocIds(reply);
   ASSERT_EQ(4, ids.size());
   for (auto id : ids) {
      EXPECT_NE(std::string::npos, id.first.find("8f8411f5-899a-445a-8421-210157db0512"));
   }

   ElasticSearch restQuery(transport, false);
   std::vector<std::pair<std::string, std::string> > idsFromESObject;
   ASSERT_TRUE(restQuery.Initialize());
   EXPECT_TRUE(restQuery.RunQueryGetIds("indexType", "foo:bar", idsFromESObject));
   EXPECT_EQ(ids, idsFromESObject);
}

TEST_F(RESTBuilderTest, ConstructAIdQueryNothingFound) {
   RESTBuilder builder;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender;

   std::string command = builder.RunQueryOnlyForDocIds("indexType", "foo:bar");

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar&cache=false|{ \"fields\" : [], \"size\" : 40}", command);

   transport.mReturnString = "200|ok|{\"took\":8,\"timed_out\":false,\"_shards\":"
           "{\"total\":50,\"successful\":50,\"failed\":0},\"hits\":{\"total\":0,\"max_score\":null,\"hits\":[]}}";
   std::string reply;
   reply = transport.Send(command);
   std::vector<std::pair<std::string, std::string> > ids = sender.GetDocIds(reply);
   ASSERT_EQ(0, ids.size());

   ElasticSearch restQuery(transport, false);
   ASSERT_TRUE(restQuery.Initialize());
   std::vector<std::pair<std::string, std::string> > idsFromESObject;
   EXPECT_TRUE(restQuery.RunQueryGetIds("indexType", "foo:bar", idsFromESObject));
   EXPECT_EQ(ids, idsFromESObject);
}

TEST_F(RESTBuilderTest, ConstructAIdQueryTimedOut) {
   RESTBuilder builder;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender();

   std::string command = builder.RunQueryOnlyForDocIds("indexType", "foo:bar");

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar&cache=false|{ \"fields\" : [], \"size\" : 40}", command);

   transport.mReturnString = "503|timeout|{\"took\":10,\"timed_out\":true,\"_shards\":{\"total\":50,"
           "\"successful\":50,\"failed\":0},\"hits\":{\"total\":4,\"max_score\":12.653517,"
           "\"hits\":[{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_2\",\"_score\":12.653517},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_3\",\"_score\":12.650981},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_4\",\"_score\":12.650732},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_1\",\"_score\":12.649386}]}}";
   std::string reply;
   reply = transport.Send(command);

   ElasticSearch restQuery(transport, false);
   std::vector<std::pair<std::string, std::string> > idsFromESObject;
   EXPECT_FALSE(restQuery.RunQueryGetIds("indexType", "foo:bar", idsFromESObject));
   ASSERT_EQ(0, idsFromESObject.size());
}

TEST_F(RESTBuilderTest, Construct) {
   MockBoomStick transport("tcp://127.0.0.1:9700");

   RESTBuilder builder;
   RESTBuilder* pBuilder = new RESTBuilder;
   delete pBuilder;

   RESTParser sender;
   RESTParser* pSender = new RESTParser;
   delete pSender;
}

TEST_F(RESTBuilderTest, CreateAndDeleteIndex) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockBoomStick transport2("tcp://127.0.0.1:9700");
   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTParser sender;
   ElasticSearch restQuery(transport2, false);
   ASSERT_TRUE(restQuery.Initialize());
   int shards(3);
   int replicas(5);
   std::string command = builder.GetIndexCreation("indexName", shards, replicas);

   EXPECT_EQ("PUT|/indexName/|{\"settings\":{\"index\":{\"number_of_shards\":3,\"number_of_replicas\":5}}}", command);
   std::string reply;
   transport2.mReturnString = transport.mReturnString = "200|ok|{\"ok\":true,\"acknowledged\":true}";
   reply = transport.Send(command);
   EXPECT_EQ("200|ok|{\"ok\":true,\"acknowledged\":true}", reply);
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_TRUE(restQuery.CreateIndex("indexName", shards, replicas));

   transport2.mReturnString = transport.mReturnString = "{\"error\":\"IndexAlreadyExistsException[[indexName] Alread exists]\",\"status\":400}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_FALSE(restQuery.CreateIndex("indexName", shards, replicas));
   EXPECT_EQ(transport.mReturnString, reply);
   transport.mReturnString.clear();
   transport2.mReturnString = transport.mReturnString = "200|ok|{\"ok\":true,\"acknowledged\":true}";

   command = builder.GetIndexDeletion("indexName");

   EXPECT_EQ("DELETE|/indexName/", command);
   reply = transport.Send(command);
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_TRUE(restQuery.DeleteIndex("indexName"));

   EXPECT_EQ("200|ok|{\"ok\":true,\"acknowledged\":true}", reply);
   transport2.mReturnString = transport.mReturnString = "404|missing|{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_FALSE(restQuery.DeleteIndex("indexName"));
   EXPECT_EQ(transport.mReturnString, reply);
}

TEST_F(RESTBuilderTest, OpenAndCloseIndex) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockBoomStick transport2("tcp://127.0.0.1:9700");
   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTParser sender;
   ElasticSearch restQuery(transport2, false);
   ASSERT_TRUE(restQuery.Initialize());
   int shards(3);
   int replicas(5);
   std::string command = builder.GetIndexClose("indexName");

   EXPECT_EQ("POST|/indexName/_close", command);
   std::string reply;
   transport2.mReturnString = transport.mReturnString = "200|ok|{\"ok\":true,\"acknowledged\":true}";
   reply = transport.Send(command);
   EXPECT_EQ("200|ok|{\"ok\":true,\"acknowledged\":true}", reply);
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_TRUE(restQuery.IndexClose("indexName"));

   transport2.mReturnString = transport.mReturnString = "404|missing|{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_FALSE(restQuery.IndexClose("indexName"));

   EXPECT_EQ(transport.mReturnString, reply);
   transport.mReturnString.clear();
   command = builder.GetIndexOpen("indexName");

   EXPECT_EQ("POST|/indexName/_open", command);
   transport2.mReturnString = transport.mReturnString = "200|ok|{\"ok\":true,\"acknowledged\":true}";
   reply = transport.Send(command);
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_TRUE(restQuery.IndexOpen("indexName"));

   EXPECT_EQ("200|ok|{\"ok\":true,\"acknowledged\":true}", reply);
   transport2.mReturnString = transport.mReturnString = "404|missing|{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_FALSE(restQuery.IndexOpen("indexName"));
   EXPECT_EQ(transport.mReturnString, reply);
}

TEST_F(RESTBuilderTest, AddDocUpdateDocDeleteDoc) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockBoomStick transport2("tcp://127.0.0.1:9700");
   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTParser sender;
   ElasticSearch es(transport2, true);
   ASSERT_TRUE(es.Initialize());
   ElasticSearch esSync(transport, false);
   ASSERT_TRUE(esSync.Initialize());
   int shards(3);
   int replicas(5);
   std::string command = builder.GetAddDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}");
   
   EXPECT_EQ("PUT|/indexName/typeName/abc_123|{\"test\":\"data\"}", command);
   std::string reply;
   transport.mReturnString = "200|ok|{\"ok\":true,\"_index\":\"indexName\",\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";

   reply = transport.Send(command);
   EXPECT_EQ(transport.mReturnString, reply);

   EXPECT_TRUE(es.AddDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}"));

   transport.mReturnString = "404|missing|{{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOK(reply));
   EXPECT_EQ(transport.mReturnString, reply);
   EXPECT_TRUE(es.AddDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}"));
   transport.mReturnString.clear();
   
   command = builder.GetUpdateDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}");
   EXPECT_EQ("POST|/indexName/typeName/abc_123/_update|{ \"doc\":{\"test\":\"data\"}}", command);
   transport.mReturnString = "200|ok|{\"ok\":true,\"_index\":\"indexName\",\"_type\":\"typeName\","
           "\"_id\":\"abc_123\",\"_version\":3}";
   reply = transport.Send(command);
   EXPECT_EQ(transport.mReturnString, reply);
   EXPECT_TRUE(esSync.UpdateDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}"));
   transport.mReturnString = "404|missing|{\"error\":\"DocumentMissingException[[indexName][4] [typeName][abc_123]: document missing]\",\"status\":404}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOK(reply));
   EXPECT_EQ(transport.mReturnString, reply);
   EXPECT_FALSE(esSync.UpdateDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}"));
   
   command = builder.GetDeleteDoc("indexName", "typeName", "abc_123");

   EXPECT_EQ("DELETE|/indexName/typeName/abc_123", command);
   transport.mReturnString = "200|ok|{\"ok\":true,\"found\":true,\"_index\":\"indexName\","
           "\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";

   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_TRUE(sender.IsOkAndFound(reply));
   EXPECT_TRUE(es.DeleteDoc("indexName", "typeName", "abc_123"));
   transport.mReturnString = "404|missing|{\"ok\":true,\"found\":false,\"_index\":\"indexName\","
           "\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_TRUE(es.DeleteDoc("indexName", "typeName", "abc_123"));
   transport.mReturnString = "404|missing|{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   reply = transport.Send(command);
   EXPECT_FALSE(sender.IsOK(reply));
   EXPECT_EQ(transport.mReturnString, reply);
   EXPECT_TRUE(es.DeleteDoc("indexName", "typeName", "abc_123"));
}

TEST_F(RESTBuilderTest, GetListOfIndexeNames) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockBoomStick transport2("tcp://127.0.0.1:9700");
   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTParser sender;
   std::string reply;

   std::string command = builder.GetIndexList();
   EXPECT_EQ(command, "GET|/_stats");
   std::fstream goodResult("resources/goodClusterStatus", std::ios_base::in);
   goodResult >> transport.mReturnString;
   goodResult.close();
   ASSERT_FALSE(transport.mReturnString.empty());

   reply = transport.Send(command);

   std::set<std::string> indexes = sender.GetOrderedListOfIndexes(reply);

   ASSERT_EQ(10, indexes.size());
   auto iterator = indexes.begin();
   EXPECT_EQ("kibana-int", *iterator++);
   EXPECT_EQ("network_1999_01_01", *iterator++);
   EXPECT_EQ("network_2012_12_31", *iterator++);
   EXPECT_EQ("network_2013_01_01", *iterator++);
   EXPECT_EQ("network_2013_07_04", *iterator++);
   EXPECT_EQ("network_2013_07_05", *iterator++);
   EXPECT_EQ("network_2013_12_31", *iterator++);
   EXPECT_EQ("network_2014_01_01", *iterator++);
   EXPECT_EQ("network_2100_12_31", *iterator++);
   EXPECT_EQ("twitter", *iterator++);

   ElasticSearch restQuery(transport2, false);
   transport2.mReturnString = transport.mReturnString;
   ASSERT_TRUE(restQuery.Initialize());
   std::set<std::string> indexes2 = restQuery.GetListOfIndexeNames();
   EXPECT_EQ(indexes, indexes2);
}

#else 

TEST_F(RESTBuilderTest, emptyTest) {
   EXPECT_TRUE(true);
}
#endif