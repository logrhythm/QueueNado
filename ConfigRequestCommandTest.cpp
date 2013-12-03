#include "ConfigRequestCommandTest.h"
#include "ConfigRequestCommand.h"
#include "MockConfigRequestCommand.h"
#include "ConfigDefaultsRequest.pb.h"
#include "ConfigDefaults.pb.h"
#include "include/global.h"
#include <boost/lexical_cast.hpp>

TEST_F(ConfigRequestCommandTest, DoesItCompileAndLink) {
#ifdef LR_DEBUG
   protoMsg::CommandRequest request;
   MockConfigRequestCommand mockDummy(request, nullptr);
   GMockConfigRequestCommand gmockDummy(request, nullptr);
   SUCCEED();
#endif
}

TEST_F(ConfigRequestCommandTest, BaseConfOneSetOfValues) {
#ifdef LR_DEBUG
   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockProcessManagerCommand testProcessor(mockConf);
   EXPECT_TRUE(testProcessor.Initialize());

   // Pretend UI. Ask for config for the dpiThreads
   protoMsg::ConfigDefaultsRequest request;
   request.set_type(protoMsg::ConfType::BASE);
   request.add_requestedconfigparams("dpiThreads");
   cmd.set_stringargone(request.SerializeAsString());
   MockConfigRequestCommand doIt(cmd, autoManagedManager);


   // Test Mock Preparation to Answer the UI
   MockConfigRequestCommand::ManyConfDefaults defaults;
   auto dpiThreads = std::make_tuple("dpiThreads", "7", MockConfigRequestCommand::Ranges{"1", "12"});
   defaults.push_back(dpiThreads);
   doIt.EnabledMockExecuteRequest(protoMsg::ConfType::BASE, defaults);


   // Fake server side. Execute the request to retrieve the Conf values
   auto reply = doIt.Execute(mockConf);
   EXPECT_TRUE(reply.success());


   // Pretend UI to verify the received Conf values
   protoMsg::ConfigDefaults realReply;
   EXPECT_TRUE(reply.has_result());
   EXPECT_TRUE(realReply.ParseFromString(reply.result()));

   // Validate that we got back values
   EXPECT_EQ(realReply.values_size(), 1);
   const auto& defaultValues = realReply.values(0);
   EXPECT_TRUE(defaultValues.has_type());
   EXPECT_TRUE(defaultValues.has_configname());
   EXPECT_TRUE(defaultValues.has_defaultu64());
   EXPECT_TRUE(defaultValues.has_maxu64());
   EXPECT_TRUE(defaultValues.has_minu64());

   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(defaultValues.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(defaultValues.configname(), "dpiThreads");
   EXPECT_EQ(defaultValues.defaultu64(), "7");
   EXPECT_EQ(defaultValues.minu64(), "1");
   EXPECT_EQ(defaultValues.maxu64(), "12");
#endif
}

TEST_F(ConfigRequestCommandTest, BaseConfTwoSetOfValues) {
#ifdef LR_DEBUG
   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockProcessManagerCommand testProcessor(mockConf);
   EXPECT_TRUE(testProcessor.Initialize());

   protoMsg::ConfigDefaultsRequest request;
   request.set_type(protoMsg::ConfType::BASE);
   request.add_requestedconfigparams("dpiThreads");
   request.add_requestedconfigparams("qosmosDebugModeEnabled");

   cmd.set_stringargone(request.SerializeAsString());
   MockConfigRequestCommand doIt(cmd, autoManagedManager);


   // BUILD UP THE FAKE RESPONSE 
   MockConfigRequestCommand::ManyConfDefaults defaults;
   auto dpiThreads = std::make_tuple("dpiThreads", "7", MockConfigRequestCommand::Ranges{"1", "12"});
   auto qosmosDebugEnabled = std::make_tuple("qosmosDebugModeEnabled", "false", MockConfigRequestCommand::Ranges{"false", "true"});
   defaults.push_back(dpiThreads);
   defaults.push_back(qosmosDebugEnabled);
   doIt.EnabledMockExecuteRequest(protoMsg::ConfType::BASE, defaults);



   auto reply = doIt.Execute(mockConf);
   EXPECT_TRUE(reply.success());

   protoMsg::ConfigDefaults realReply;
   EXPECT_TRUE(reply.has_result());
   EXPECT_TRUE(realReply.ParseFromString(reply.result()));

   // Validate that we got back values
   EXPECT_EQ(realReply.values_size(), 2);
   const auto& readDpiValues = realReply.values(0);
   EXPECT_TRUE(readDpiValues.has_type());
   EXPECT_TRUE(readDpiValues.has_configname());
   EXPECT_TRUE(readDpiValues.has_defaultu64());
   EXPECT_TRUE(readDpiValues.has_maxu64());
   EXPECT_TRUE(readDpiValues.has_minu64());
   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(readDpiValues.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(readDpiValues.configname(), "dpiThreads");
   EXPECT_EQ(readDpiValues.defaultu64(), "7");
   EXPECT_EQ(readDpiValues.minu64(), "1");
   EXPECT_EQ(readDpiValues.maxu64(), "12");

   const auto& readQosmosDebug = realReply.values(1);
   EXPECT_TRUE(readQosmosDebug.has_type());
   EXPECT_TRUE(readQosmosDebug.has_configname());
   EXPECT_TRUE(readQosmosDebug.has_defaultu64());
   EXPECT_TRUE(readQosmosDebug.has_maxu64());
   EXPECT_TRUE(readQosmosDebug.has_minu64());
   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(readQosmosDebug.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(readQosmosDebug.configname(), "qosmosDebugModeEnabled");
   EXPECT_EQ(readQosmosDebug.defaultu64(), "false");
   EXPECT_EQ(readQosmosDebug.minu64(), "false");
   EXPECT_EQ(readQosmosDebug.maxu64(), "true");
#endif
}

TEST_F(ConfigRequestCommandTest, DISABLED_BaseConfAllValues) {
}

// some confs that we could add tests for either here or elsewhere

TEST_F(ConfigRequestCommandTest, DISABLED_QosmosConf) {
}

TEST_F(ConfigRequestCommandTest, DISABLED_SyslogConf) {
}

TEST_F(ConfigRequestCommandTest, DISABLED_NTP) {
}

TEST_F(ConfigRequestCommandTest, DISABLED_etcetcetc) {
}

// test with zero params,. always failure
// test with one params,.. should work
// test with multiple params ,... should work
// test with non_existent params should fail


