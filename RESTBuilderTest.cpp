#include "RESTBuilderTest.h"
#include "MockBoomStick.h"

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

   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTSender sender(transport);
   int shards(3);
   int replicas(5);
   std::string command = builder.GetIndexCreation("indexName", shards, replicas);

   EXPECT_EQ("PUT|/indexName/|{\"settings\":{\"index\":{\"number_of_shards\":3,\"number_of_replicas\":5}}}", command);
   std::string reply;
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   transport.mReturnString = "{\"error\":\"IndexAlreadyExistsException[[indexName] Alread exists]\",\"status\":400}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_EQ(transport.mReturnString, reply);
   transport.mReturnString.clear();
   command = builder.GetIndexDeletion("indexName");

   EXPECT_EQ("DELETE|/indexName/", command);
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_EQ(transport.mReturnString, reply);
}

TEST_F(RESTBuilderTest, OpenAndCloseIndex) {
   MockBoomStick transport("tcp://127.0.0.1:9700");

   ASSERT_TRUE(transport.Initialize());
   RESTBuilder builder;
   RESTSender sender(transport);
   int shards(3);
   int replicas(5);
   std::string command = builder.GetIndexClose("indexName");

   EXPECT_EQ("POST|/indexName/_close", command);
   std::string reply;
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_EQ(transport.mReturnString, reply);
   transport.mReturnString.clear();
   command = builder.GetIndexOpen("indexName");

   EXPECT_EQ("POST|/indexName/_open", command);
   EXPECT_TRUE(sender.Send(command, reply));
   EXPECT_EQ("{\"ok\":true,\"acknowledged\":true}", reply);
   transport.mReturnString = "{\"error\":\"IndexMissingException[[indexName] missing]\",\"status\":404}";
   EXPECT_FALSE(sender.Send(command, reply));
   EXPECT_EQ(transport.mReturnString, reply);
}