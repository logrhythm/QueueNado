#include <fstream>
#include "ElasticSearchTest.h"
#include "MockElasticSearch.h"
#include "include/global.h"
#include "MockBoomStick.h"


#ifdef LR_DEBUG

TEST_F(ElasticSearchTest, SynchrnousCannotAddDel) {
   MockBoomStick transport("tcp://127.0.0.1:9700");
   transport.mReturnString = "{\"ok\":true,\"_index\":\"indexName\",\"_type\":\"typeName\",\"_id\":\"abc_123\",\"_version\":1}";

   ElasticSearch es(transport, false);
   ASSERT_TRUE(es.Initialize());
   EXPECT_FALSE(es.AddDoc("foo", "bar", "baz", "fuz"));
   EXPECT_FALSE(es.DeleteDoc("foo", "bar", "baz"));
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

   MessageIdentifier id;
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
