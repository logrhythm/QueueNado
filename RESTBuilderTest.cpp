#include "RESTBuilderTest.h"
#include "MockBoomStick.h"
#include "ElasticSearch.h"
#include <unordered_set>
#include <fstream>

#ifdef LR_DEBUG

TEST_F(RESTBuilderTest, ConstructAQuery) {
   RESTBuilder builder;
   
   std::string command = builder.RunQueryOnAllIndicies("indexType", "foo:bar");

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar", command);

   
}
TEST_F(RESTBuilderTest, ConstructAIdQuery) {
   RESTBuilder builder;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTSender sender(transport);
   
   std::string command = builder.RunQueryOnlyForDocIds("indexType", "foo:bar");

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar|{ \"fields\" : [] }", command);

   transport.mReturnString = "{\"took\":10,\"timed_out\":false,\"_shards\":{\"total\":50,"
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
   ASSERT_TRUE(sender.Send(command,reply));
   std::vector<std::pair<std::string, std::string> > ids = sender.GetDocIds(reply);
   ASSERT_EQ(4,ids.size());
   for (auto id : ids) {
      EXPECT_NE(std::string::npos,id.first.find("8f8411f5-899a-445a-8421-210157db0512"));
   }
   
   ElasticSearch restQuery(transport,false);
   std::vector<std::pair<std::string, std::string> > idsFromESObject = restQuery.RunQueryGetIds("indexType", "foo:bar");
   EXPECT_EQ(ids,idsFromESObject);
}
TEST_F(RESTBuilderTest, ConstructAIdQueryNothingFound) {
   RESTBuilder builder;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTSender sender(transport);
   
   std::string command = builder.RunQueryOnlyForDocIds("indexType", "foo:bar");

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar|{ \"fields\" : [] }", command);

   transport.mReturnString = "{\"took\":8,\"timed_out\":false,\"_shards\":"
           "{\"total\":50,\"successful\":50,\"failed\":0},\"hits\":{\"total\":0,\"max_score\":null,\"hits\":[]}}";
   std::string reply;
   ASSERT_TRUE(sender.Send(command,reply));
   std::vector<std::pair<std::string, std::string> > ids = sender.GetDocIds(reply);
   ASSERT_EQ(0,ids.size());

   ElasticSearch restQuery(transport,false);
   std::vector<std::pair<std::string, std::string> > idsFromESObject = restQuery.RunQueryGetIds("indexType", "foo:bar");
   EXPECT_EQ(ids,idsFromESObject);
}
TEST_F(RESTBuilderTest, ConstructAIdQueryTimedOut) {
   RESTBuilder builder;
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTSender sender(transport);
   
   std::string command = builder.RunQueryOnlyForDocIds("indexType", "foo:bar");

   EXPECT_EQ("GET|/_all/indexType/_search?q=foo:bar|{ \"fields\" : [] }", command);

   transport.mReturnString = "{\"took\":10,\"timed_out\":true,\"_shards\":{\"total\":50,"
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
   ASSERT_FALSE(sender.Send(command,reply));

   ElasticSearch restQuery(transport,false);
   std::vector<std::pair<std::string, std::string> > idsFromESObject = restQuery.RunQueryGetIds("indexType", "foo:bar");
   ASSERT_EQ(0,idsFromESObject.size());
}
TEST_F(RESTBuilderTest, Construct) {
   MockBoomStick transport("tcp://127.0.0.1:9700");

   RESTBuilder builder;
   RESTBuilder* pBuilder = new RESTBuilder;
   delete pBuilder;

   RESTSender sender(transport);
   RESTSender* pSender = new RESTSender(transport);
   delete pSender;
}

TEST_F(RESTBuilderTest, CreateAndDeleteIndex) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockBoomStick transport2("tcp://127.0.0.1:9700");
   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTSender sender(transport);
   ElasticSearch restQuery(transport2,false);
   ASSERT_TRUE(restQuery.Initialize());
   int shards(3);
   int replicas(5);
   std::string command = builder.GetIndexCreation("indexName", shards, replicas);

   EXPECT_EQ("PUT|/indexName/|{\"settings\":{\"index\":{\"number_of_shards\":3,\"number_of_replicas\":5}}}", command);
   std::string reply;
   transport2.mReturnString = transport.mReturnString = "{\"ok\":true,\"acknowledged\":true}";
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_TRUE(restQuery.CreateIndex("indexName", shards, replicas));

   transport2.mReturnString = transport.mReturnString = "{\"error\":\"IndexAlreadyExistsException[[indexName] Alread exists]\",\"status\":400}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_FALSE(restQuery.CreateIndex("indexName", shards, replicas));
   EXPECT_EQ(transport.mReturnString, reply);
   transport.mReturnString.clear();
   transport2.mReturnString = transport.mReturnString = "{\"ok\":true,\"acknowledged\":true}";

   command = builder.GetIndexDeletion("indexName");

   EXPECT_EQ("DELETE|/indexName/", command);
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_TRUE(restQuery.DeleteIndex("indexName"));

   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   transport2.mReturnString = transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
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
   RESTSender sender(transport);
   ElasticSearch restQuery(transport2,false);
   ASSERT_TRUE(restQuery.Initialize());
   int shards(3);
   int replicas(5);
   std::string command = builder.GetIndexClose("indexName");

   EXPECT_EQ("POST|/indexName/_close", command);
   std::string reply;
   transport2.mReturnString = transport.mReturnString = "{\"ok\":true,\"acknowledged\":true}";
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_TRUE(restQuery.IndexClose("indexName"));

   transport2.mReturnString = transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_FALSE(restQuery.IndexClose("indexName"));

   EXPECT_EQ(transport.mReturnString, reply);
   transport.mReturnString.clear();
   command = builder.GetIndexOpen("indexName");

   EXPECT_EQ("POST|/indexName/_open", command);
   transport2.mReturnString = transport.mReturnString = "{\"ok\":true,\"acknowledged\":true}";
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_TRUE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));

   EXPECT_TRUE(restQuery.IndexOpen("indexName"));

   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   transport2.mReturnString = transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_FALSE(restQuery.IndexOpen("indexName"));
   EXPECT_EQ(transport.mReturnString, reply);
}

TEST_F(RESTBuilderTest, AddDocDeleteDoc) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockBoomStick transport2("tcp://127.0.0.1:9700");
   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTSender sender(transport);
   ElasticSearch es(transport2,true);
   ASSERT_TRUE(es.Initialize());
   int shards(3);
   int replicas(5);
   std::string command = builder.GetAddDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}");

   EXPECT_EQ("PUT|/indexName/typeName/abc_123|{\"test\":\"data\"}", command);
   std::string reply;
   transport.mReturnString = "{\"ok\":true,\"_index\":\"indexName\",\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";

   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_EQ(transport.mReturnString, reply);

   EXPECT_TRUE(es.AddDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}"));

   transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_EQ(transport.mReturnString, reply);
   EXPECT_TRUE(es.AddDoc("indexName", "typeName", "abc_123", "{\"test\":\"data\"}"));
   transport.mReturnString.clear();

   command = builder.GetDeleteDoc("indexName", "typeName", "abc_123");

   EXPECT_EQ("DELETE|/indexName/typeName/abc_123", command);
   transport.mReturnString = "{\"ok\":true,\"found\":true,\"_index\":\"indexName\","
           "\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";

   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_TRUE(sender.IsOkAndFound(reply));
   EXPECT_TRUE(es.DeleteDoc("indexName", "typeName", "abc_123"));
   transport.mReturnString = "{\"ok\":true,\"found\":false,\"_index\":\"indexName\","
           "\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_FALSE(sender.IsOkAndAck(reply));
   EXPECT_FALSE(sender.IsOkAndFound(reply));
   EXPECT_TRUE(es.DeleteDoc("indexName", "typeName", "abc_123"));
   transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_EQ(transport.mReturnString, reply);
   EXPECT_TRUE(es.DeleteDoc("indexName", "typeName", "abc_123"));
}

TEST_F(RESTBuilderTest, GetListOfIndexeNames) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockBoomStick transport2("tcp://127.0.0.1:9700");
   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTSender sender(transport);
   std::string reply;

   std::string command = builder.GetIndexList();
   EXPECT_EQ(command, "GET|/_stats");
   std::fstream goodResult("resources/goodClusterStatus", std::ios_base::in);
   goodResult >> transport.mReturnString;
   goodResult.close();
   ASSERT_FALSE(transport.mReturnString.empty());
   
   EXPECT_TRUE(sender.Send(command, reply));

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

   ElasticSearch restQuery(transport2,false);
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