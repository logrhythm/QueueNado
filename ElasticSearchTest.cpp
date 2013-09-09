#include <fstream>
#include "ElasticSearchTest.h"
#include "MockElasticSearch.h"
#include "include/global.h"
#include "MockBoomStick.h"

#include "MockSkelleton.h"
#ifdef LR_DEBUG

TEST_F(ElasticSearchTest, ValgrindTestSyncRunQueryGetIds) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   target.BeginListenAndRepeat();
   
   int count = 0;
   target.mReplyMessage = "{\"took\":10,\"timed_out\":false,\"_shards\":{\"total\":50,"
           "\"successful\":50,\"failed\":0},\"hits\":{\"total\":4,\"max_score\":12.653517,"
           "\"hits\":[{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_2\",\"_score\":12.653517},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_3\",\"_score\":12.650981},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_4\",\"_score\":12.650732},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_1\",\"_score\":12.649386}]}}";
   std::vector<std::pair<std::string, std::string> > recordsToUpdate;
   while (count ++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.RunQueryGetIds("meta", "foo: bar", recordsToUpdate));
      EXPECT_FALSE(recordsToUpdate.empty());
   }
   target.mReplyMessage = "{\"took\":8,\"timed_out\":false,\"_shards\":"
           "{\"total\":50,\"successful\":50,\"failed\":0},\"hits\":{\"total\":0,\"max_score\":null,\"hits\":[]}}";
   count = 0;
   while (count ++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.RunQueryGetIds("meta", "foo: bar", recordsToUpdate));
      EXPECT_TRUE(recordsToUpdate.empty());
   }
   count = 0;
   target.mReplyMessage = "{\"took\":10,\"timed_out\":true,\"_shards\":{\"total\":50,"
           "\"successful\":50,\"failed\":0},\"hits\":{\"total\":4,\"max_score\":12.653517,"
           "\"hits\":[{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_2\",\"_score\":12.653517},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_3\",\"_score\":12.650981},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_4\",\"_score\":12.650732},"
           "{\"_index\":\"network_2013_08_12\",\"_type\":\"meta\","
           "\"_id\":\"8f8411f5-899a-445a-8421-210157db0512_1\",\"_score\":12.649386}]}}";
   while (count ++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.RunQueryGetIds("meta", "foo: bar", recordsToUpdate));
      EXPECT_TRUE(recordsToUpdate.empty());
   }
}

TEST_F(ElasticSearchTest, ValgrindTestSyncGetListOfIndexeNames) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   std::set<std::string> indexNames;
   std::fstream goodResult("resources/goodClusterStatus", std::ios_base::in);
   goodResult >> target.mReplyMessage;
   goodResult.close();
   while (count ++ < targetIterations && !zctx_interrupted) {
      indexNames = es.GetListOfIndexeNames();
   }
}

TEST_F(ElasticSearchTest, ValgrindTestSyncCreateIndex) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"acknowledged\":true}";
   while (count ++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.CreateIndex("test", 1, 1));
   }
   target.mReplyMessage = "{\"error\":\"IndexAlreadyExistsException[[indexName] Alread exists]\",\"status\":400}";
   while (count ++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.CreateIndex("test", 1, 1));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestSyncDeleteIndex) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"acknowledged\":true}";
   while (count ++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.DeleteIndex("test"));
   }
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count ++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.DeleteIndex("test"));
   }
}
   //   LR_VIRTUAL bool IndexClose(const std::string& indexName);
   //   LR_VIRTUAL bool IndexOpen(const std::string& indexName);
   //   LR_VIRTUAL bool AddDoc(const std::string& indexName, const std::string& indexType, const std::string& id, const std::string& jsonData);
   //   LR_VIRTUAL bool UpdateDoc(const std::string& indexName, const std::string& indexType, const std::string& id, const std::string& jsonData);
   //   LR_VIRTUAL bool DeleteDoc(const std::string& indexName, const std::string& indexType, const std::string& id);
   //   bool RefreshDiskInfo();
   //   std::vector<std::string> GetClusterNames();
   //   uint64_t GetTotalWrites(const std::string& clusterName);
   //   uint64_t GetTotalReads(const std::string& clusterName);
   //   uint64_t GetTotalWriteBytes(const std::string& clusterName);
   //   uint64_t GetTotalReadBytes(const std::string& clusterName);

TEST_F(ElasticSearchTest, ValgrindTestAsync) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, true);
   target.mReplyMessage = "{\"ok\":true,\"acknowledged\":true}";
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   target.BeginListenAndRepeat();


}

TEST_F(ElasticSearchTest, AsynchrnousCannotDoOtherStuff) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   ElasticSearch es(transport, true);

   std::fstream goodResult("resources/goodClusterStatus", std::ios_base::in);
   goodResult >> transport.mReturnString;
   goodResult.close();
   ASSERT_FALSE(transport.mReturnString.empty());
   ASSERT_TRUE(es.Initialize());
   std::set<std::string> indexList = es.GetListOfIndexeNames();
   EXPECT_TRUE(indexList.empty());

   transport.mReturnString = "{\"ok\":true,\"acknowledged\":true}";
   EXPECT_FALSE(es.CreateIndex("foo", 1, 1));
   EXPECT_FALSE(es.DeleteIndex("foo"));
   EXPECT_FALSE(es.IndexClose("foo"));
   EXPECT_FALSE(es.IndexOpen("foo"));

   transport.mReturnString = "{\"ok\":true,\"_index\":\"indexName\",\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";
   EXPECT_TRUE(es.AddDoc("foo", "bar", "baz", "fuz"));
   EXPECT_TRUE(es.DeleteDoc("foo", "bar", "baz"));
}

TEST_F(ElasticSearchTest, ReplyNotReadyIsFalseBadIsTrue) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   ElasticSearchSocket es(transport, true);

   std::string id = transport.GetUuid();
   std::string reply;
   transport.mReturnString = "FAILFAILFAIL";
   transport.mReturnSocketEmpty = true;
   EXPECT_FALSE(es.AttemptToGetReplyFromSearch(id, reply));
   transport.mReturnSocketEmpty = false;
   transport.mReturnString = "FAILFAILFAIL";
   EXPECT_TRUE(es.AttemptToGetReplyFromSearch(id, reply));
   transport.mReturnString = "\"ok\":true";
   EXPECT_TRUE(es.AttemptToGetReplyFromSearch(id, reply));
}

TEST_F(ElasticSearchTest, SyncSendsReplies) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);

   std::fstream goodResult("resources/goodClusterStatus", std::ios_base::in);
   goodResult >> transport.mReturnString;
   goodResult.close();
   ASSERT_FALSE(transport.mReturnString.empty());
   ASSERT_TRUE(es.Initialize());
   es.mFakeIndexList = false;
   std::set<std::string> indexList = es.GetListOfIndexeNames();
   EXPECT_TRUE(es.ReplySent());
   es.ReplySet(false);

   transport.mReturnString = "{\"ok\":true,\"acknowledged\":true}";
   EXPECT_TRUE(es.CreateIndex("foo", 1, 1));
   EXPECT_TRUE(es.ReplySent());
   es.ReplySet(false);
   es.mFakeDeleteIndex = false;
   EXPECT_TRUE(es.DeleteIndex("foo"));
   EXPECT_TRUE(es.ReplySent());
   es.ReplySet(false);
   EXPECT_TRUE(es.IndexClose("foo"));
   EXPECT_TRUE(es.ReplySent());
   es.ReplySet(false);
   EXPECT_TRUE(es.IndexOpen("foo"));
   EXPECT_TRUE(es.ReplySent());
   es.ReplySet(false);

}

TEST_F(ElasticSearchTest, AsyncDoesnotSendReplies) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, true);

   transport.mReturnString = "{\"ok\":true,\"_index\":\"indexName\",\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";

   ASSERT_TRUE(es.Initialize());
   EXPECT_TRUE(es.AddDoc("foo", "bar", "baz", "fuz"));
   EXPECT_FALSE(es.ReplySent());
   EXPECT_TRUE(es.DeleteDoc("foo", "bar", "baz"));
   EXPECT_FALSE(es.ReplySent());
}

#else

TEST_F(ElasticSearchTest, empty) {
   EXPECT_TRUE(true);
}
#endif
