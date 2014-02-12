#include "ConfigRequestCommandTest.h"
#include "ConfigRequestCommand.h"
#include "MockConfigRequestCommand.h"
#include "ConfigDefaultsRequest.pb.h"
#include "ConfigDefaults.pb.h"
#include "include/global.h"
#include "ConfSlave.h"
#include "PcapDiskUsage.h"
#include "ConfTypeMsg.pb.h"
#include "ProcessManager.h"
#include <boost/lexical_cast.hpp>

TEST_F(ConfigRequestCommandTest, DoesItCompileAndLink) {
#ifdef LR_DEBUG
   protoMsg::CommandRequest request;
   ProcessClient processClient(mockConf);
   MockConfigRequestCommand mockDummy(request, processClient);
   GMockConfigRequestCommand gmockDummy(request, processClient);
   SUCCEED();
#endif
}

TEST_F(ConfigRequestCommandTest, BaseConfOneSetOfValues) {
#ifdef LR_DEBUG
   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockProcessClientCommand testProcessor(mockConf);
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
   EXPECT_TRUE(defaultValues.has_defaultval());
   EXPECT_TRUE(defaultValues.has_max());
   EXPECT_TRUE(defaultValues.has_min());

   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(defaultValues.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(defaultValues.configname(), "dpiThreads");
   EXPECT_EQ(defaultValues.defaultval(), "7");
   EXPECT_EQ(defaultValues.min(), "1");
   EXPECT_EQ(defaultValues.max(), "12");
#endif
}

TEST_F(ConfigRequestCommandTest, BaseConfTwoSetOfValues) {
#ifdef LR_DEBUG
   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockProcessClientCommand testProcessor(mockConf);
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
   EXPECT_TRUE(readDpiValues.has_defaultval());
   EXPECT_TRUE(readDpiValues.has_max());
   EXPECT_TRUE(readDpiValues.has_min());
   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(readDpiValues.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(readDpiValues.configname(), "dpiThreads");
   EXPECT_EQ(readDpiValues.defaultval(), "7");
   EXPECT_EQ(readDpiValues.min(), "1");
   EXPECT_EQ(readDpiValues.max(), "12");

   const auto& readQosmosDebug = realReply.values(1);
   EXPECT_TRUE(readQosmosDebug.has_type());
   EXPECT_TRUE(readQosmosDebug.has_configname());
   EXPECT_TRUE(readQosmosDebug.has_defaultval());
   EXPECT_TRUE(readQosmosDebug.has_max());
   EXPECT_TRUE(readQosmosDebug.has_min());
   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(readQosmosDebug.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(readQosmosDebug.configname(), "qosmosDebugModeEnabled");
   EXPECT_EQ(readQosmosDebug.defaultval(), "false");
   EXPECT_EQ(readQosmosDebug.min(), "false");
   EXPECT_EQ(readQosmosDebug.max(), "true");
#endif
}

TEST_F(ConfigRequestCommandTest, BaseConfExecuteUsingRealDefaultValues) {
#ifdef LR_DEBUG

   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockConfMaster confMaster;
   ProcessManager::InstanceWithConfMaster(confMaster);
   MockProcessClientCommand testProcessor(confMaster.GetConf());
   EXPECT_TRUE(testProcessor.Initialize());

   protoMsg::ConfigDefaultsRequest request;
   request.set_type(protoMsg::ConfType::BASE);
   request.add_requestedconfigparams("dpiThreads");
   request.add_requestedconfigparams("qosmosDebugModeEnabled");

   cmd.set_stringargone(request.SerializeAsString());
   MockConfigRequestCommand doIt(cmd, autoManagedManager);

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
   EXPECT_TRUE(readDpiValues.has_defaultval());
   EXPECT_TRUE(readDpiValues.has_max());
   EXPECT_TRUE(readDpiValues.has_min());
   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(readDpiValues.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(readDpiValues.configname(), "dpiThreads");
   EXPECT_EQ(readDpiValues.defaultval(), "4");
   EXPECT_EQ(readDpiValues.min(), "1");
   EXPECT_EQ(readDpiValues.max(), "12");

   const auto& readQosmosDebug = realReply.values(1);
   EXPECT_TRUE(readQosmosDebug.has_type());
   EXPECT_TRUE(readQosmosDebug.has_configname());
   EXPECT_TRUE(readQosmosDebug.has_defaultval());
   EXPECT_TRUE(readQosmosDebug.has_max());
   EXPECT_TRUE(readQosmosDebug.has_min());
   // check that the values are as expected for the dpithreads as set above
   EXPECT_EQ(readQosmosDebug.type(), protoMsg::ConfType::BASE);
   EXPECT_EQ(readQosmosDebug.configname(), "qosmosDebugModeEnabled");
   EXPECT_EQ(readQosmosDebug.defaultval(), "false");
   EXPECT_EQ(readQosmosDebug.min(), "false");
   EXPECT_EQ(readQosmosDebug.max(), "true");
#endif
}

void checkRealReply(protoMsg::ConfigDefaults realReply,
        std::string configName,
        protoMsg::ConfType::Type type,
        std::string defaultVal,
        std::string min,
        std::string max,
        int it) {
   const auto& readReply = realReply.values(it);
   EXPECT_TRUE(readReply.has_type());
   EXPECT_TRUE(readReply.has_configname());
   EXPECT_TRUE(readReply.has_defaultval());
   EXPECT_TRUE(readReply.has_max());
   EXPECT_TRUE(readReply.has_min());

   EXPECT_EQ(readReply.type(), type);
   EXPECT_EQ(readReply.configname(), configName);
   EXPECT_EQ(readReply.defaultval(), defaultVal);
   EXPECT_EQ(readReply.min(), min);
   EXPECT_EQ(readReply.max(), max) << configName;
}

TEST_F(ConfigRequestCommandTest, BaseConfAllValues) {
#ifdef LR_DEBUG
   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockProcessClientCommand testProcessor(mockConf);
   EXPECT_TRUE(testProcessor.Initialize());

   protoMsg::ConfigDefaultsRequest request;
   request.set_type(protoMsg::ConfType::BASE);
   request.add_requestedconfigparams("dpiThreads");
   request.add_requestedconfigparams("pcapETimeout");
   request.add_requestedconfigparams("pcapBufferSize");
   request.add_requestedconfigparams("pcapInterface");
   request.add_requestedconfigparams("dpiHalfSessions");
   request.add_requestedconfigparams("packetSendQueueSize");
   request.add_requestedconfigparams("packetRecvQueueSize");
   request.add_requestedconfigparams("dpiMsgSendQueueSize");
   request.add_requestedconfigparams("dpiMsgRecvQueueSize");
   request.add_requestedconfigparams("syslogSendQueueSize");
   request.add_requestedconfigparams("syslogRecvQueueSize");
   request.add_requestedconfigparams("qosmosDebugModeEnabled");
   request.add_requestedconfigparams("qosmos64BytePool");
   request.add_requestedconfigparams("qosmos128BytePool");
   request.add_requestedconfigparams("qosmos256BytePool");
   request.add_requestedconfigparams("qosmos512BytePool");
   request.add_requestedconfigparams("statsAccumulatorQueue");
   request.add_requestedconfigparams("sendStatsQueue");
   request.add_requestedconfigparams("qosmosExpirePerCallback");
   request.add_requestedconfigparams("qosmosTCPReAssemblyEnabled");
   request.add_requestedconfigparams("qosmosIPDefragmentationEnabled");
   request.add_requestedconfigparams("dbClusterName");
   request.add_requestedconfigparams("dbUrl");
   request.add_requestedconfigparams("statsAggregationQueue");
   request.add_requestedconfigparams("commandQueue");
   request.add_requestedconfigparams("enableIntermediateFlows");
   request.add_requestedconfigparams("enablePacketCapture");
   request.add_requestedconfigparams("captureFileLimit");
   request.add_requestedconfigparams("captureSizeLimit");
   request.add_requestedconfigparams("captureMemoryLimit");
   request.add_requestedconfigparams("captureMaxPackets");
   request.add_requestedconfigparams("captureIndividualFileLimit");
   request.add_requestedconfigparams("dpiMaxFree");
   request.add_requestedconfigparams("dpiMinFree");
   request.add_requestedconfigparams("dpiFreeThreshold");
   request.add_requestedconfigparams("dpiRecycleThreshold");
   request.add_requestedconfigparams("pcapRecordsToClearPerCycle");

   cmd.set_stringargone(request.SerializeAsString());
   MockConfigRequestCommand doIt(cmd, autoManagedManager);

   auto reply = doIt.Execute(mockConf);
   EXPECT_TRUE(reply.success());

   protoMsg::ConfigDefaults realReply;
   EXPECT_TRUE(reply.has_result());
   EXPECT_TRUE(realReply.ParseFromString(reply.result()));

   int it = 0;
   checkRealReply(realReply, "dpiThreads", protoMsg::ConfType::BASE, "4", "1", "12", it++);
   checkRealReply(realReply, "pcapETimeout", protoMsg::ConfType::BASE, "1", "1", "30", it++);
   checkRealReply(realReply, "pcapBufferSize", protoMsg::ConfType::BASE, "35", "1", "1000", it++);
   checkRealReply(realReply, "pcapInterface", protoMsg::ConfType::BASE, "em2", "1", "1000", it++);
   checkRealReply(realReply, "dpiHalfSessions", protoMsg::ConfType::BASE, "1500000", "1000000", "20000000", it++);
   checkRealReply(realReply, "packetSendQueueSize", protoMsg::ConfType::BASE, "800", "10", "10000", it++);
   checkRealReply(realReply, "packetRecvQueueSize", protoMsg::ConfType::BASE, "800", "10", "10000", it++);
   checkRealReply(realReply, "dpiMsgSendQueueSize", protoMsg::ConfType::BASE, "10000", "10", "100000", it++);
   checkRealReply(realReply, "dpiMsgRecvQueueSize", protoMsg::ConfType::BASE, "10000", "10", "100000", it++);
   checkRealReply(realReply, "syslogSendQueueSize", protoMsg::ConfType::BASE, "1000", "10", "10000", it++);
   checkRealReply(realReply, "syslogRecvQueueSize", protoMsg::ConfType::BASE, "1000", "10", "10000", it++);
   checkRealReply(realReply, "qosmosDebugModeEnabled", protoMsg::ConfType::BASE, "false", "false", "true", it++);
   checkRealReply(realReply, "qosmos64BytePool", protoMsg::ConfType::BASE, "1500000", "200000", "8000000", it++);
   checkRealReply(realReply, "qosmos128BytePool", protoMsg::ConfType::BASE, "3000000", "200000", "8000000", it++);
   checkRealReply(realReply, "qosmos256BytePool", protoMsg::ConfType::BASE, "500000", "200000", "8000000", it++);
   checkRealReply(realReply, "qosmos512BytePool", protoMsg::ConfType::BASE, "500000", "200000", "800000", it++);
   checkRealReply(realReply, "statsAccumulatorQueue", protoMsg::ConfType::BASE, "ipc:///tmp/statsAccumulatorQ.ipc", "6", "1000", it++);
   checkRealReply(realReply, "sendStatsQueue", protoMsg::ConfType::BASE, "ipc:///tmp/statsmsg.ipc", "6", "1000", it++);
   checkRealReply(realReply, "qosmosExpirePerCallback", protoMsg::ConfType::BASE, "1", "1", "200", it++);
   checkRealReply(realReply, "qosmosTCPReAssemblyEnabled", protoMsg::ConfType::BASE, "true", "false", "true", it++);
   checkRealReply(realReply, "qosmosIPDefragmentationEnabled", protoMsg::ConfType::BASE, "true", "false", "true", it++);
   checkRealReply(realReply, "dbClusterName", protoMsg::ConfType::BASE, "TestCluster", "1", "1000", it++);
   checkRealReply(realReply, "dbUrl", protoMsg::ConfType::BASE, "localhost:9160", "3", "1000", it++);
   checkRealReply(realReply, "statsAggregationQueue", protoMsg::ConfType::BASE, "ipc:///tmp/statsAccumulatorQ.ipc", "6", "1000", it++);
   checkRealReply(realReply, "commandQueue", protoMsg::ConfType::BASE, "tcp://127.0.0.1:5556", "6", "1000", it++);
   checkRealReply(realReply, "enableIntermediateFlows", protoMsg::ConfType::BASE, "true", "false", "true", it++);
   checkRealReply(realReply, "enablePacketCapture", protoMsg::ConfType::BASE, "true", "false", "true", it++);
   checkRealReply(realReply, "captureFileLimit", protoMsg::ConfType::BASE, "1000000", "1000", std::to_string(std::numeric_limits<int32_t>::max()), it++);
   
   
   
   ProtoDefaults getDefaults{mockConf.GetPcapCaptureLocations()};
   auto confDefaults = getDefaults.GetConfDefaults(protoMsg::ConfType_Type_BASE);
   auto rangePtr = getDefaults.GetRange(confDefaults, "captureSizeLimit"); // int  
   
   checkRealReply(realReply, "captureSizeLimit", protoMsg::ConfType::BASE, "80000", "1000", rangePtr->StringifyMax(), it++);
   checkRealReply(realReply, "captureMemoryLimit", protoMsg::ConfType::BASE, "1500", "1000", "16000", it++);
   checkRealReply(realReply, "captureMaxPackets", protoMsg::ConfType::BASE, "2000000000", "1000", std::to_string(std::numeric_limits<int32_t>::max()), it++);
   checkRealReply(realReply, "captureIndividualFileLimit", protoMsg::ConfType::BASE, "2000", "1", "2000000", it++);
   checkRealReply(realReply, "dpiMaxFree", protoMsg::ConfType::BASE, "2000", "1", std::to_string(std::numeric_limits<int32_t>::max()), it++);
   checkRealReply(realReply, "dpiMinFree", protoMsg::ConfType::BASE, "200", "1", std::to_string(std::numeric_limits<int32_t>::max()), it++);
   checkRealReply(realReply, "dpiFreeThreshold", protoMsg::ConfType::BASE, "0.01", std::to_string(0.0), std::to_string(1.0), it++);
   checkRealReply(realReply, "dpiRecycleThreshold", protoMsg::ConfType::BASE, "1048576", "1024", "33554432", it++);
   checkRealReply(realReply, "pcapRecordsToClearPerCycle", protoMsg::ConfType::BASE, "10000", "1", "20000", it++);
#endif
}

TEST_F(ConfigRequestCommandTest, SyslogConfAllValues) {
#ifdef LR_DEBUG
   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockProcessClientCommand testProcessor(mockConf);
   EXPECT_TRUE(testProcessor.Initialize());

   protoMsg::ConfigDefaultsRequest request;
   request.set_type(protoMsg::ConfType::SYSLOG);
   request.add_requestedconfigparams("syslogEnabled");
   request.add_requestedconfigparams("syslogTcpEnabled");
   request.add_requestedconfigparams("syslogLogAgentPort");
   request.add_requestedconfigparams("syslogLogAgentIP");
   request.add_requestedconfigparams("syslogMaxLineLength");
   request.add_requestedconfigparams("siemLogging");
   request.add_requestedconfigparams("debugSiemLogging");
   //request.add_requestedconfigparams("reportEverything");
   request.add_requestedconfigparams("scrubPasswords");

   cmd.set_stringargone(request.SerializeAsString());
   MockConfigRequestCommand doIt(cmd, autoManagedManager);

   auto reply = doIt.Execute(mockConf);
   EXPECT_TRUE(reply.success());

   protoMsg::ConfigDefaults realReply;
   EXPECT_TRUE(reply.has_result());
   EXPECT_TRUE(realReply.ParseFromString(reply.result()));

   int it = 0;
   checkRealReply(realReply, "syslogEnabled", protoMsg::ConfType::SYSLOG, "true", "false", "true", it++);
   checkRealReply(realReply, "syslogTcpEnabled", protoMsg::ConfType::SYSLOG, "true", "false", "true", it++);
   checkRealReply(realReply, "syslogLogAgentPort", protoMsg::ConfType::SYSLOG, "514", "1", "65535", it++);
   checkRealReply(realReply, "syslogLogAgentIP", protoMsg::ConfType::SYSLOG, "0.0.0.0", "1", "1000", it++);
   checkRealReply(realReply, "syslogMaxLineLength", protoMsg::ConfType::SYSLOG, "1999", "200", "2000", it++);
   checkRealReply(realReply, "siemLogging", protoMsg::ConfType::SYSLOG, "true", "false", "true", it++);
   checkRealReply(realReply, "debugSiemLogging", protoMsg::ConfType::SYSLOG, "true", "false", "true", it++);
   //checkRealReply(realReply, "reportEverything", protoMsg::ConfType::SYSLOG, "false", "false", "true", it++);
   checkRealReply(realReply, "scrubPasswords", protoMsg::ConfType::SYSLOG, "false", "false", "true", it++);
#endif
}

// some confs that we could add tests for either here or elsewhere

TEST_F(ConfigRequestCommandTest, DISABLED_QosmosConf) {
}

TEST_F(ConfigRequestCommandTest, DISABLED_NTP) {
}

TEST_F(ConfigRequestCommandTest, DISABLED_etcetcetc) {
}

// test with zero params,. always failure
// test with one params,.. should work
// test with multiple params ,... should work
// test with non_existent params should fail


