#include <fstream>
#include "ElasticSearchTest.h"
#include "MockElasticSearch.h"
#include "include/global.h"
#include "MockBoomStick.h"

#include "MockSkelleton.h"
#ifdef LR_DEBUG



TEST_F(ElasticSearchTest, TransportCannotInit) {
   MockBoomStick stick{mAddress};
   stick.mFailsInit = true;
   ElasticSearch es(stick,false);
   ASSERT_FALSE(es.Initialize());
   
}

TEST_F(ElasticSearchTest, TransportCannotCreateContext) {
   MockBoomStick stick{mAddress};
   stick.mFailsGetNewContext = true;
   ElasticSearch es(stick,false);
   ASSERT_FALSE(es.Initialize());
   
}

TEST_F(ElasticSearchTest, SendAndForgetFailures) {
 
   MockBoomStick stick{mAddress};
   MockElasticSearch es1(stick,false);
   ASSERT_TRUE(es1.Initialize());
   ASSERT_FALSE(es1.SendAndForgetCommandToWorker("foo"));
   MockElasticSearch es2(stick,true);
   ASSERT_FALSE(es2.SendAndForgetCommandToWorker("bar"));
}


TEST_F(ElasticSearchTest, GetListOfIndexeNamesFailures) {
 
   MockBoomStick stick{mAddress};
   MockElasticSearch es1(stick,true);
   es1.mFakeIndexList = false;
   ASSERT_TRUE(es1.Initialize());
   std::set<std::string> returnSet;
   returnSet.insert("removeMe");
   returnSet = es1.GetListOfIndexeNames();
   ASSERT_TRUE(returnSet.empty());
   returnSet.insert("removeMe");
   MockElasticSearch es2(stick,false);
   es2.mFakeIndexList = false;
   returnSet = es2.GetListOfIndexeNames();
   ASSERT_TRUE(returnSet.empty());
}

TEST_F(ElasticSearchTest, BulkUpdate) {
 
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   
   IdsAndIndexes ids;
   
   ids.emplace_back("test1","testa");
   ids.emplace_back("test2","testb");
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   ASSERT_TRUE(es.BulkUpdate(ids,"testing","{\"this\":\"that\"}"));
   EXPECT_EQ("POST|/_bulk|"
   "{ \"update\" : { \"_id\" : \"test1\", \"_type\" : \"testing\", \"_index\" : \"testa\"}}\n"
   "{ \"doc\" : {\"this\":\"that\"}}\n"
   "{ \"update\" : { \"_id\" : \"test2\", \"_type\" : \"testing\", \"_index\" : \"testb\"}}\n"
   "{ \"doc\" : {\"this\":\"that\"}}\n"
           ,target.mLastRequest);
   
   
}
TEST_F(ElasticSearchTest, BulkUpdateFailures) {
   IdsAndIndexes ids;
   ids.emplace_back("test1","testa");
   ids.emplace_back("test2","testb");
   MockBoomStick stick{mAddress};
   MockElasticSearch es2(stick,false);
   es2.mFakeBulkUpdate = false;
   EXPECT_FALSE(es2.BulkUpdate(ids,"testing","{\"this\":\"that\"}"));
}
TEST_F(ElasticSearchTest, DeleteDocFailures) {

   MockBoomStick stick{mAddress};
   MockElasticSearch es2(stick,false);
   
   EXPECT_FALSE(es2.DeleteDoc("test","testing","123456789012345678901234567890123456"));
}
TEST_F(ElasticSearchTest, DeleteDoc) {
 
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   
   IdsAndIndexes ids;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   ASSERT_TRUE(es.DeleteDoc("test","testing","123456789012345678901234567890123456"));
   EXPECT_EQ("DELETE|/test/testing/123456789012345678901234567890123456"
           ,target.mLastRequest);
   
}
TEST_F(ElasticSearchTest, RefreshDiskInfoFailures) {

   MockBoomStick stick{mAddress};
   MockElasticSearch es2(stick,false);
   EXPECT_FALSE(es2.RefreshDiskInfo());
   MockElasticSearch es1(stick,true);
   ASSERT_TRUE(es1.Initialize());
   es1.BoostDiskInfo(); // to exercise a log path
   EXPECT_FALSE(es1.RefreshDiskInfo());

}
TEST_F(ElasticSearchTest, RefreshDiskInfo) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();

   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123}]}}}}";
   
   ASSERT_TRUE(es.RefreshDiskInfo());
   
}

TEST_F(ElasticSearchTest, GetClusterNames) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();

   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123}]}}}}";
   std::vector<std::string> clusterNames = es.GetClusterNames();
   EXPECT_TRUE(clusterNames.empty());
   ASSERT_TRUE(es.RefreshDiskInfo());
   clusterNames = es.GetClusterNames();
   ASSERT_EQ(1,clusterNames.size());
   EXPECT_EQ("foo",clusterNames[0]);
   
}
TEST_F(ElasticSearchTest, GetTotalWrites) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();

   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123, \"disk_writes\": 12345}]}}}}";
   uint64_t clusterWrites= es.GetTotalWrites("foo");
   EXPECT_EQ(0,clusterWrites);
   ASSERT_TRUE(es.RefreshDiskInfo());
   clusterWrites = es.GetTotalWrites("foo");
   ASSERT_EQ(12345,clusterWrites);
   
}
TEST_F(ElasticSearchTest, GetTotalReads) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();

   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123, \"disk_reads\": 12345}]}}}}";
   uint64_t clusterWrites= es.GetTotalReads("foo");
   EXPECT_EQ(0,clusterWrites);
   ASSERT_TRUE(es.RefreshDiskInfo());
   clusterWrites = es.GetTotalReads("foo");
   ASSERT_EQ(12345,clusterWrites);
   
}
TEST_F(ElasticSearchTest, GetTotalWriteBytes) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();

   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123, \"disk_write_size_in_bytes\": 12345}]}}}}";
   uint64_t clusterWrites= es.GetTotalWriteBytes("foo");
   EXPECT_EQ(0,clusterWrites);
   ASSERT_TRUE(es.RefreshDiskInfo());
   clusterWrites = es.GetTotalWriteBytes("foo");
   ASSERT_EQ(12345,clusterWrites);
   
}
TEST_F(ElasticSearchTest, GetTotalReadBytes) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();

   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123, \"disk_read_size_in_bytes\": 12345}]}}}}";
   uint64_t clusterWrites= es.GetTotalReadBytes("foo");
   EXPECT_EQ(0,clusterWrites);
   ASSERT_TRUE(es.RefreshDiskInfo());
   clusterWrites = es.GetTotalReadBytes("foo");
   ASSERT_EQ(12345,clusterWrites);
   
}
TEST_F(ElasticSearchTest, ValgrindTestSyncAddDoc) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", "{something: true}"));
   }
   count = 0;
   target.mDrowzy = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", "{something: true}"));
   }
   count = 0;
   target.mDrowzy = false;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", "{something: true}"));
   }
   target.mReplyMessage.clear();
   target.mEmptyReplies = true;
   count = 0;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", "{something: true}"));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncAddDoc) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, true);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   count = 0;
   target.mDrowzy = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   count = 0;
   target.mDrowzy = false;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   target.mReplyMessage.clear();
   target.mEmptyReplies = true;
   count = 0;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.AddDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestSyncUpdateDoc) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   count = 0;
   target.mDrowzy = true;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   count = 0;
   target.mDrowzy = false;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   target.mReplyMessage.clear();
   target.mEmptyReplies = true;
   count = 0;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncUpdateDoc) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, true);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   count = 0;
   target.mDrowzy = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   count = 0;
   target.mDrowzy = false;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
   target.mReplyMessage.clear();
   target.mEmptyReplies = true;
   count = 0;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.UpdateDoc("test", "meta", "123456789012345678901234567890123456", mBigRecord));
   }
}

TEST_F(ElasticSearchTest, g2LogMemoryGrowTest) {
   int count = 0;

   while (count++ < targetIterations && !zctx_interrupted) {
      LOG(DEBUG) << "Lets fill up memory !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
   }
}

TEST_F(ElasticSearchTest, ValgrindTestSyncRunQueryGetIds) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();

   int count = 0;
   std::vector<std::pair<std::string, std::string> > recordsToUpdate;
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

   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.RunQueryGetIds("meta", "foo: bar", recordsToUpdate));
      EXPECT_FALSE(recordsToUpdate.empty());
   }
   target.mReplyMessage = "{\"took\":8,\"timed_out\":false,\"_shards\":"
           "{\"total\":50,\"successful\":50,\"failed\":0},\"hits\":{\"total\":0,\"max_score\":null,\"hits\":[]}}";
   count = 0;
   while (count++ < targetIterations && !zctx_interrupted) {
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
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.RunQueryGetIds("meta", "foo: bar", recordsToUpdate));
      EXPECT_TRUE(recordsToUpdate.empty());
   }
   target.mReplyMessage.clear();
   target.mEmptyReplies = true;
   count = 0;
   while (count++ < targetIterations && !zctx_interrupted) {
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
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   std::set<std::string> indexNames;
   std::fstream goodResult("resources/goodClusterStatus", std::ios_base::in);
   goodResult >> target.mReplyMessage;
   goodResult.close();
   while (count++ < targetIterations && !zctx_interrupted) {
      indexNames = es.GetListOfIndexeNames();
      EXPECT_FALSE(indexNames.empty());
   }
}

TEST_F(ElasticSearchTest, ValgrindTestSyncCreateIndex) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   //   target.mReplyMessage = "{\"ok\":true,\"acknowledged\":true}";
   //   while (count ++ < targetIterations && ! zctx_interrupted) {
   //      EXPECT_TRUE(es.CreateIndex("test", 1, 1));
   //   }
   //   count = 0;
   //   target.mReplyMessage = "{\"error\":\"IndexAlreadyExistsException[[indexName] Alread exists]\",\"status\":400}";
   //   while (count ++ < targetIterations && ! zctx_interrupted) {
   //      EXPECT_FALSE(es.CreateIndex("test", 1, 1));
   //   }
   //   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.CreateIndex("test", 1, 1));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncCreateIndex) {
   // not supported in Sync
   EXPECT_TRUE(true);
}

TEST_F(ElasticSearchTest, ValgrindTestSyncDeleteIndex) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"acknowledged\":true}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.DeleteIndex("test"));
   }
   count = 0;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.DeleteIndex("test"));
   }
   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.DeleteIndex("test"));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncDeleteIndex) {
   // not supported in Sync
   EXPECT_TRUE(true);
}

TEST_F(ElasticSearchTest, ValgrindTestSyncIndexClose) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"acknowledged\":true}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.IndexClose("test"));
   }
   count = 0;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.IndexClose("test"));
   }
   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.IndexClose("test"));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncIndexClose) {
   // not supported in Sync
   EXPECT_TRUE(true);
}

TEST_F(ElasticSearchTest, ValgrindTestSyncIndexOpen) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"acknowledged\":true}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.IndexOpen("test"));
   }
   count = 0;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.IndexOpen("test"));
   }
   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.IndexOpen("test"));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncIndexOpen) {
   // not supported in Sync
   EXPECT_TRUE(true);
}

TEST_F(ElasticSearchTest, ValgrindTestSyncDeleteDoc) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.DeleteDoc("test", "meta", "123456789012345678901234567890123456"));
   }
   count = 0;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.DeleteDoc("test", "meta", "123456789012345678901234567890123456"));
   }
   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.DeleteDoc("test", "meta", "123456789012345678901234567890123456"));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncDeleteDoc) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, true);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"ok\":true,\"_index\":\"test\",\"_type\":\"meta\",\"_id\":\"123456789012345678901234567890123456\",\"_version\":1}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.DeleteDoc("test", "meta", "123456789012345678901234567890123456"));
   }
   count = 0;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.DeleteDoc("test", "meta", "123456789012345678901234567890123456"));
   }
   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.DeleteDoc("test", "meta", "123456789012345678901234567890123456"));
   }
}

TEST_F(ElasticSearchTest, ValgrindTestSyncRefreshDiskInfo) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123}]}}}}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.RefreshDiskInfo());
   }
   count = 0;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.RefreshDiskInfo());
   }
   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.RefreshDiskInfo());
   }
}

TEST_F(ElasticSearchTest, ValgrindTestASyncRefreshDiskInfo) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   int count = 0;
   target.mReplyMessage = "{\"nodes\": {\"disk1\":{\"name\": \"foo\", \"fs\" : { \"data\": [{\"foo\": 123}]}}}}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_TRUE(es.RefreshDiskInfo());
   }
   count = 0;
   target.mReplyMessage = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.RefreshDiskInfo());
   }
   target.mReplyMessage.clear();
   count = 0;
   target.mEmptyReplies = true;
   while (count++ < targetIterations && !zctx_interrupted) {
      EXPECT_FALSE(es.RefreshDiskInfo());
   }
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

TEST_F(ElasticSearchTest, RunQueryGetIds) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);
   const std::string indexType("foo");
   const std::string query("bar");
   std::vector<std::pair<std::string, std::string> > recordsToUpdate;
   const int recordsToQuery(1000);
   const bool cache(true);

   es.mRealSendAndGetReplyCommandToWorker = false;
   es.mReturnSendAndGetReplyCommandToWorker = true;
   es.mSendAndGetReplyReply = "{\"ok\":true,\"timed_out\":false}";

   EXPECT_TRUE(es.RunQueryGetIds(indexType, query, recordsToUpdate, recordsToQuery, cache));

   es.mSendAndGetReplyReply = "{\"ok\":true,\"timed_out\":true}";
   EXPECT_FALSE(es.RunQueryGetIds(indexType, query, recordsToUpdate, recordsToQuery, cache));

   es.mSendAndGetReplyReply = "BAD_REQUEST";
   EXPECT_TRUE(es.RunQueryGetIds(indexType, query, recordsToUpdate, recordsToQuery, cache));
}

TEST_F(ElasticSearchTest, RunQueryGetIdsFailed) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);
   const std::string indexType("foo");
   const std::string query("bar");
   std::vector<std::pair<std::string, std::string> > recordsToUpdate;
   const int recordsToQuery(1000);
   const bool cache(true);

   es.mRealSendAndGetReplyCommandToWorker = false;
   es.mReturnSendAndGetReplyCommandToWorker = false;
   EXPECT_FALSE(es.RunQueryGetIds(indexType, query, recordsToUpdate, recordsToQuery, cache));
}

TEST_F(ElasticSearchTest, GetOldestNFilesFailed) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);
   std::vector<std::tuple< std::string, std::string> > oldestFiles;
   const unsigned int numberOfFiles(100);
   const std::string path("/tmp");
   IdsAndIndexes relevantRecords;
   time_t oldestTime = 123456789;
   es.mRealSendAndGetReplyCommandToWorker = false;
   es.mReturnSendAndGetReplyCommandToWorker = false;
   es.mFakeGetOldestNFiles = false;
   oldestFiles.emplace_back("foo", "bar");
   relevantRecords.emplace_back("foo", "bar");
   oldestFiles = es.GetOldestNFiles(numberOfFiles,path,relevantRecords,oldestTime);
   EXPECT_EQ(0,oldestTime);
   EXPECT_TRUE(oldestFiles.empty());
   EXPECT_TRUE(relevantRecords.empty());
   
}
TEST_F(ElasticSearchTest, GetOldestNFiles) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);
   std::vector<std::tuple< std::string, std::string> > oldestFiles;
   const unsigned int numberOfFiles(100);
   const std::string path("/tmp");
   IdsAndIndexes relevantRecords;
   time_t oldestTime = 123456789;
   es.mRealSendAndGetReplyCommandToWorker = false;
   es.mReturnSendAndGetReplyCommandToWorker = true;
   es.mFakeGetOldestNFiles = false;
   es.mSendAndGetReplyReply = "{\"ok\":true,\"timed_out\":false}";
   oldestFiles.emplace_back("foo", "bar");
   relevantRecords.emplace_back("foo", "bar");
   oldestFiles = es.GetOldestNFiles(numberOfFiles,path,relevantRecords,oldestTime);
   EXPECT_EQ(0,oldestTime);
   EXPECT_TRUE(oldestFiles.empty());
   EXPECT_TRUE(relevantRecords.empty());
   
   es.mSendAndGetReplyReply.clear();
   oldestFiles.emplace_back("foo", "bar");
   relevantRecords.emplace_back("foo", "bar");
   oldestFiles = es.GetOldestNFiles(numberOfFiles,path,relevantRecords,oldestTime);
   EXPECT_EQ(0,oldestTime);
   EXPECT_TRUE(oldestFiles.empty());
   EXPECT_TRUE(relevantRecords.empty());
}
TEST_F(ElasticSearchTest, GetDiskInfo) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);
}

TEST_F(ElasticSearchTest, DocCommandAsync) {

   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, true);

   es.mReturnSendAndForgetCommandToWorker = true;
   es.mRealSendAndForgetCommandToWorker = false;
   EXPECT_TRUE(es.DocCommand("foo"));
   EXPECT_FALSE(es.mRanSendAndGetReplyCommandToWorker);
   EXPECT_TRUE(es.mRanSendAndForgetCommandToWorker);

}

TEST_F(ElasticSearchTest, DocCommandAsyncFails) {

   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, true);

   es.mReturnSendAndForgetCommandToWorker = false;
   es.mRealSendAndForgetCommandToWorker = false;
   EXPECT_FALSE(es.DocCommand("foo"));
   EXPECT_FALSE(es.mRanSendAndGetReplyCommandToWorker);
   EXPECT_TRUE(es.mRanSendAndForgetCommandToWorker);

}

TEST_F(ElasticSearchTest, DocCommandSyncFails) {

   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);
   es.mRealSendAndGetReplyCommandToWorker = false;
   es.mReturnSendAndGetReplyCommandToWorker = false;

   EXPECT_FALSE(es.DocCommand("foo"));
   EXPECT_TRUE(es.mRanSendAndGetReplyCommandToWorker);
   EXPECT_FALSE(es.mRanSendAndForgetCommandToWorker);
}

TEST_F(ElasticSearchTest, DocCommandSync) {

   MockBoomStick transport("tcp://127.0.0.1:9700");
   MockElasticSearch es(transport, false);
   es.mRealSendAndGetReplyCommandToWorker = false;
   es.mReturnSendAndGetReplyCommandToWorker = true;
   es.mSendAndGetReplyReply = "{\"ok\":true,\"timed_out\":false}";

   EXPECT_TRUE(es.DocCommand("foo"));
   EXPECT_TRUE(es.mRanSendAndGetReplyCommandToWorker);
   EXPECT_FALSE(es.mRanSendAndForgetCommandToWorker);
   es.mSendAndGetReplyReply = "{\"ok\":false,\"timed_out\":false}";
   EXPECT_FALSE(es.DocCommand("foo"));
   es.mSendAndGetReplyReply = "{\"ok\":true,\"timed_out\":true}";
}
TEST_F(ElasticSearchTest, DoNothingFor31Seconds) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   ElasticSearch es(stick, false);
   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   ASSERT_TRUE(es.Initialize());
   target.BeginListenAndRepeat();
   
   std::this_thread::sleep_for(std::chrono::seconds(31));
   
}

#else

TEST_F(ElasticSearchTest, empty) {
   EXPECT_TRUE(true);
}
#endif
