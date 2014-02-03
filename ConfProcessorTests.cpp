/*
 * File:   ConfProcessorTests.cpp
 * Author: Ben Aldrich
 *
 * Created on September 28, 2012, 3:53 PM
 */

#include "ConfNetInterface.h"
#include "ConfProcessorTests.h"
#include "ConfMaster.h"
#include "ConfSlave.h"
#include "Conf.h"
#include "ConfTypeMsg.pb.h"
#include "Headcrab.h"
#include "Crowbar.h"
#include "libconf/Conf.h"
#include "MockConfSlave.h"
#include "MockConfMaster.h"
#include "RestartMsg.pb.h"
#include "VersionMsg.pb.h"
#include "ShutdownMsg.pb.h"
#include "SyslogConfMsg.pb.h"
#include "Shotgun.h"
#include "g2log.hpp"
#include "QosmosProtocolCapture.h"
#include <stdio.h>
using namespace std;

#ifdef LR_DEBUG
#include "MockConf.h"
#include "MockEthInfo.h"


/**
 * Found a bug that when calling ConfSlave::Instance and get conf I could 
 * potentially get defaults instead of actually getting the conf.
 * @param 
 */
TEST_F(ConfProcessorTests, EmptySetGetActualConf) {
   Conf conf = ConfSlave::Instance().GetConf();
   ASSERT_EQ(conf.GetPath(), "conf/nm.yaml");

   conf = ConfMaster::Instance().GetConf();
   ASSERT_EQ(conf.GetPath(), "conf/nm.yaml");
}

TEST_F(ConfProcessorTests, EthConfValidate) {
   MockConf conf;
   conf.mValidateEthFailCount = -1;
   MockEthInfo ethInfo;
   ConfMap protoMap;
   ethInfo.mFakeIsValid = true;
   ethInfo.mFakeIsValidValue = false;
   EXPECT_TRUE(conf.ValidateEthConfFields(protoMap, ethInfo)); // Cannot verify anything

   ethInfo.mFakeInitialize = true;
   ethInfo.mFakeInitializeFailure = false;
   ethInfo.mFakeInterfaceNames.insert("test1");
   ethInfo.mFakeInterfaceNames.insert("test2");
   ethInfo.mFakeInterfaceNames.insert("test3");

   ethInfo.Initialize();
   ethInfo.mFakeIsValidValue = true;
   EXPECT_FALSE(conf.ValidateEthConfFields(protoMap, ethInfo)); // "" value is false

   conf.mPCAPInterface = "false";
   EXPECT_FALSE(conf.ValidateEthConfFields(protoMap, ethInfo)); // not in valid interface names

   conf.mPCAPInterface = "test1";
   EXPECT_TRUE(conf.ValidateEthConfFields(protoMap, ethInfo));
}

TEST_F(ConfProcessorTests, EthConfRepair) {
   MockConf conf;
   conf.mValidateEthFailCount = -1;
   MockEthInfo ethInfo;
   ConfMap protoMap;

   ethInfo.mFakeInitialize = true;
   ethInfo.mFakeInitializeFailure = false;
   ethInfo.mFakeInterfaceNames.insert("test1");
   ethInfo.mFakeInterfaceNames.insert("test2");
   ethInfo.mFakeInterfaceNames.insert("em2");

   ethInfo.Initialize();
   ethInfo.mFakeIsValid = true;
   ethInfo.mFakeIsValidValue = false;
   conf.RepairEthConfFieldsWithDefaults(protoMap, ethInfo); // doesn't do anything
   EXPECT_EQ(0, protoMap.size());
   ethInfo.mFakeIsValidValue = true;
   conf.RepairEthConfFieldsWithDefaults(protoMap, ethInfo); // empty becomes full
   ASSERT_EQ(1, protoMap.size());
   EXPECT_EQ("em2", protoMap["pcapInterface"]);
   conf.mPCAPInterface = "false";
   protoMap.clear();
   conf.RepairEthConfFieldsWithDefaults(protoMap, ethInfo); // wrong becomes right
   ASSERT_EQ(1, protoMap.size());
   EXPECT_EQ("em2", protoMap["pcapInterface"]);
   ethInfo.mFakeInterfaceNames.clear();
   ethInfo.mFakeInterfaceNames.insert("test1");
   ethInfo.mFakeInterfaceNames.insert("test2");
   ethInfo.mFakeInterfaceNames.insert("eth1");
   ethInfo.Initialize();
   conf.mPCAPInterface = "em2";
   protoMap.clear();
   conf.RepairEthConfFieldsWithDefaults(protoMap, ethInfo); // flip to eth1
   ASSERT_EQ(1, protoMap.size());
   EXPECT_EQ("eth1", protoMap["pcapInterface"]);
}

TEST_F(ConfProcessorTests, BaseConfInternalRepair) {
   MockConf conf;
   MockEthInfo ethInfo;
   conf.mValidateEthFailCount = 2;
   ethInfo.mFakeIsValid = true;
   ethInfo.mFakeIsValidValue = true;
   EXPECT_FALSE(conf.InternallyRepairBaseConf(ethInfo)); // Repair always fails
   conf.mValidateEthFailCount = 0;
   EXPECT_FALSE(conf.InternallyRepairBaseConf(ethInfo)); // no change needed
   conf.mValidateEthFailCount = 1;
   EXPECT_TRUE(conf.InternallyRepairBaseConf(ethInfo)); // repair works
}
#endif

TEST_F(ConfProcessorTests, ConfInterfaceInitialize) {
   ConfNetInterface conf;
   ASSERT_EQ("conf/nm.yaml.Interface", conf.GetPath());
   EXPECT_EQ(0, conf.GetMethod()); //desfault 
}

TEST_F(ConfProcessorTests, ConfInterfaceInitializeWithPath) {
   ConfNetInterface conf("/tmp/path/does/not/exist.conf");
   ASSERT_EQ("/tmp/path/does/not/exist.conf", conf.GetPath());
   EXPECT_EQ(0, conf.GetMethod()); //default
   EXPECT_TRUE(conf.GetInterface().empty());
}

TEST_F(ConfProcessorTests, ConfInterfaceInitializeTestSerialize) {
   ConfNetInterface conf(mTestInterfaceConf);
   EXPECT_EQ("resources/test.yaml.Interface", conf.GetPath());
   EXPECT_EQ("eth0", conf.GetInterface());
   EXPECT_EQ(1, conf.GetMethod());
   EXPECT_EQ("127.0.0.1", conf.GetIpAddress());
   EXPECT_EQ("255.255.255.0", conf.GetNetMask());
   EXPECT_EQ("127.0.0.2", conf.GetGateway());
   EXPECT_EQ("127.1.1.1, 127.3.3.3", conf.GetDnsServers());
   EXPECT_EQ("blah.domain.net, blah2.domain.net", conf.GetSearchDomains());
   EXPECT_EQ("Management Interface", conf.GetName());
}

TEST_F(ConfProcessorTests, ConfInterfaceUpdateProto) {
   ConfNetInterface conf;

   EXPECT_EQ(conf.GetMethod(), 0);

   protoMsg::NetInterface* interface = conf.getProtoMsg();
   interface->set_method(protoMsg::NetInterfaceMethod::STATICIP);
   interface->set_interface("eth1");
   interface->set_ipaddress("24.24.24.24");
   interface->set_name("name");
   interface->set_dnsservers("dns");
   interface->set_gateway("24.24.24.1");
   interface->set_netmask("255.255.255.0");
   interface->set_searchdomains("search");
   //create new object with our proto message.
   ConfNetInterface updateConf(*interface);

   EXPECT_EQ(protoMsg::NetInterfaceMethod::STATICIP, updateConf.GetMethod());
   EXPECT_EQ("eth1", updateConf.GetInterface());
   EXPECT_EQ("24.24.24.24", updateConf.GetIpAddress());
   EXPECT_EQ("name", updateConf.GetName());
   EXPECT_EQ("dns", updateConf.GetDnsServers());
   EXPECT_EQ("24.24.24.1", updateConf.GetGateway());
   EXPECT_EQ("255.255.255.0", updateConf.GetNetMask());
   EXPECT_EQ("search", updateConf.GetSearchDomains());

   protoMsg::NetInterface* updateInterface = updateConf.getProtoMsg();

   EXPECT_EQ(updateInterface->method(), protoMsg::NetInterfaceMethod::STATICIP);
   EXPECT_EQ("eth1", updateInterface->interface());
   EXPECT_EQ("24.24.24.24", updateInterface->ipaddress());
   EXPECT_EQ("name", updateInterface->name());
   EXPECT_EQ("dns", updateInterface->dnsservers());
   EXPECT_EQ("24.24.24.1", updateInterface->gateway());
   EXPECT_EQ("255.255.255.0", updateInterface->netmask());
   EXPECT_EQ("search", updateInterface->searchdomains());

   delete interface;
   delete updateInterface;
}
#ifndef LR_DEBUG

TEST_F(ConfProcessorTests, ReadConfFilesPerformance) {
   unsigned int iterations(50);
   StartTimedSection(.7, iterations);
   for (unsigned int i = 0; i < iterations; i++) {
      Conf conf;
      EXPECT_TRUE((conf.getDpiThreads() >= 1));    
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
}

/**
 * This is a performance benchmark of where we would like to get our
 * read performance to.
 * @param 
 */
TEST_F(ConfProcessorTests, DISABLED_ReadConfFilesPerformance_future) {
   unsigned int iterations(1000);
   StartTimedSection(.05, iterations);
   for (unsigned int i = 0; i < iterations; i++) {
      Conf conf;
      EXPECT_TRUE((conf.getDpiThreads() >= 1));    
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
}

TEST_F(ConfProcessorTests, ReadPerformanceBenchmark_BASE) {
   ConfMaster& master = ConfMaster::Instance();
   master.Start();
   Conf conf(master.GetConf());
   Crowbar sender(conf.getConfChangeQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::ConfType typeMsg;

   typeMsg.set_direction(protoMsg::ConfType_Direction_RECEIVING);
   typeMsg.set_type(protoMsg::ConfType_Type_BASE);
   std::vector<std::string> messages;
   messages.push_back(typeMsg.SerializeAsString());
   messages.push_back("");

   ASSERT_TRUE(sender.Flurry(messages));
   ASSERT_TRUE(sender.BlockForKill(messages));
   protoMsg::BaseConf baseMsg;
   ASSERT_TRUE(baseMsg.ParseFromString(messages[1]));
   unsigned int iterations(10000);
   StartTimedSection(.0005, iterations);
   for (unsigned int i = 0; i < iterations; i++) {
      typeMsg.set_direction(protoMsg::ConfType_Direction_RECEIVING);
      messages[0] = typeMsg.SerializeAsString();
      ASSERT_TRUE(sender.Flurry(messages));
      ASSERT_TRUE(sender.BlockForKill(messages));
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   master.Stop();
}

TEST_F(ConfProcessorTests, WritePerformanceBenchmark_BASE) {
   ConfMaster& master = ConfMaster::Instance();
   master.Start();
   Conf conf(master.GetConf());
   Crowbar sender(conf.getConfChangeQueue());
   ASSERT_TRUE(sender.Wield());
   protoMsg::ConfType typeMsg;

   typeMsg.set_direction(protoMsg::ConfType_Direction_RECEIVING);
   typeMsg.set_type(protoMsg::ConfType_Type_BASE);
   std::vector<std::string> messages;
   messages.push_back(typeMsg.SerializeAsString());
   messages.push_back("");

   ASSERT_TRUE(sender.Flurry(messages));
   ASSERT_TRUE(sender.BlockForKill(messages));
   protoMsg::BaseConf baseMsg;
   ASSERT_TRUE(baseMsg.ParseFromString(messages[1]));
   unsigned int iterations(50);
   StartTimedSection(.7, iterations);
   for (unsigned int i = 0; i < iterations; i++) {
      typeMsg.set_direction(protoMsg::ConfType_Direction_SENDING);
      messages[0] = typeMsg.SerializeAsString();
      ASSERT_TRUE(sender.Flurry(messages));
      ASSERT_TRUE(sender.BlockForKill(messages));
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   master.Stop();
}
#endif

TEST_F(ConfProcessorTests, TestProcessBaseConfigRequest) {
#ifdef LR_DEBUG
   MockConfMaster master;
   master.SetConfLocation("/tmp/dummyWriteLocationCommandProcessorTests");
   Conf conf(master.GetConf());
   master.UpdateCachedMessages(conf);
   protoMsg::ConfType configTypeMessage;
   std::string serializedConf;

   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);

   protoMsg::BaseConf baseConf;
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_TRUE(serializedConf.empty());

   baseConf.set_qosmostcpreassemblyenabled("true");
   EXPECT_TRUE(master.ProcessBaseConfigRequest(conf, baseConf.SerializeAsString()));
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   EXPECT_TRUE(baseConf.ParseFromString(serializedConf));
   EXPECT_EQ("true", baseConf.qosmostcpreassemblyenabled());

   unlink(master.mConfLocation.c_str());
#endif
}

TEST_F(ConfProcessorTests, TestProcessQosmosConfigRequest) {
#ifdef LR_DEBUG
   MockConfMaster master;
   master.SetConfLocation("/tmp/dummyWriteLocationCommandProcessorTests");
   Conf conf(master.GetConf());
   master.UpdateCachedMessages(conf);
   protoMsg::ConfType configTypeMessage;
   std::string serializedConf;

   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_QOSMOS);

   protoMsg::QosmosConf qConf;
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_TRUE(serializedConf.empty());

   protoMsg::QosmosConf_Protocol* dummyProto = qConf.add_qosmosprotocol();
   dummyProto->set_protocolenabled(false);
   dummyProto->set_protocolfamily("something");
   dummyProto->set_protocollongname("long something");
   dummyProto->set_protocolname("s");

   EXPECT_TRUE(master.ProcessQosmosConfigRequest(conf, qConf.SerializeAsString()));
   //Due to external deps, that is all we can hope for
   std::string qName = master.mConfLocation;
   qName += ".Qosmos";
   unlink(qName.c_str());
#endif  
}

TEST_F(ConfProcessorTests, TestProcessSyslogConfigRequest) {
#ifdef LR_DEBUG
   MockConfMaster master;
   master.SetConfLocation("/tmp/dummyWriteLocationCommandProcessorTests");
   Conf conf(master.GetConf());
   master.UpdateCachedMessages(conf);
   protoMsg::ConfType configTypeMessage;
   std::string serializedConf;

   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_SYSLOG);

   protoMsg::SyslogConf sConf;
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_TRUE(serializedConf.empty());

   sConf.set_siemlogging("false");
   EXPECT_TRUE(master.ProcessSyslogConfigRequest(conf, sConf.SerializeAsString()));
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   EXPECT_TRUE(sConf.ParseFromString(serializedConf));
   EXPECT_EQ("false", sConf.siemlogging());

   std::string sName = master.mConfLocation;
   sName += ".Syslog";
   unlink(sName.c_str());
#endif
}

TEST_F(ConfProcessorTests, TestIsRestartRequest) {
#ifdef LR_DEBUG
   MockConfMaster master;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_RESTART);
   EXPECT_TRUE(master.IsRestartRequest(configTypeMessage));
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_RECEIVING);
   EXPECT_TRUE(master.IsRestartRequest(configTypeMessage));
   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);
   EXPECT_FALSE(master.IsRestartRequest(configTypeMessage));
   configTypeMessage.set_type(protoMsg::ConfType_Type_QOSMOS);
   EXPECT_FALSE(master.IsRestartRequest(configTypeMessage));
   configTypeMessage.set_type(protoMsg::ConfType_Type_SYSLOG);
   EXPECT_FALSE(master.IsRestartRequest(configTypeMessage));
   configTypeMessage.set_type(protoMsg::ConfType_Type_APP_VERSION);
   EXPECT_FALSE(master.IsRestartRequest(configTypeMessage));
#endif
}

TEST_F(ConfProcessorTests, TestIsShutdownRequest) {
#ifdef LR_DEBUG
   MockConfMaster master;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_SHUTDOWN);
   EXPECT_EQ(configTypeMessage.type(), protoMsg::ConfType_Type_SHUTDOWN);
   EXPECT_TRUE(master.IsShutdownRequest(configTypeMessage));
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_RECEIVING);
   EXPECT_TRUE(master.IsShutdownRequest(configTypeMessage));

   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);
   EXPECT_FALSE(master.IsShutdownRequest(configTypeMessage));
   configTypeMessage.set_type(protoMsg::ConfType_Type_QOSMOS);
   EXPECT_FALSE(master.IsShutdownRequest(configTypeMessage));
   configTypeMessage.set_type(protoMsg::ConfType_Type_SYSLOG);
   EXPECT_FALSE(master.IsShutdownRequest(configTypeMessage));
   configTypeMessage.set_type(protoMsg::ConfType_Type_APP_VERSION);
   EXPECT_FALSE(master.IsShutdownRequest(configTypeMessage));
#endif
}

TEST_F(ConfProcessorTests, TestUpdateBaseCachedMessages) {
#ifdef LR_DEBUG
   MockConfMaster master;
   MockConf conf(master.GetConf());
   master.UpdateCachedMessages(conf);
   sleep(5);
   protoMsg::ConfType configTypeMessage;
   std::string serializedConf;

   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);

   protoMsg::BaseConf baseConf;
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   EXPECT_TRUE(baseConf.ParseFromString(serializedConf));
   EXPECT_EQ("false", baseConf.qosmostcpreassemblyenabled());

   baseConf.set_qosmostcpreassemblyenabled("true");
   conf.updateFields(baseConf);
   master.UpdateCachedMessages(conf);
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   EXPECT_TRUE(baseConf.ParseFromString(serializedConf));
   EXPECT_EQ("true", baseConf.qosmostcpreassemblyenabled());
#endif
}

TEST_F(ConfProcessorTests, TestUpdateQosmosCachedMessages) {
#ifdef LR_DEBUG
   MockConfMaster master;
   MockConf conf(master.GetConf());
   master.UpdateCachedMessages(conf);
   protoMsg::ConfType configTypeMessage;
   std::string serializedConf;
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_QOSMOS);
   EXPECT_FALSE(serializedConf.empty());
   QosmosConf qConf;
   conf.updateQosmos(qConf);
   master.UpdateCachedMessages(conf);
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_TRUE(serializedConf.empty());

#endif
}

TEST_F(ConfProcessorTests, TestUpdateSyslogCachedMessages) {
#ifdef LR_DEBUG
   MockConfMaster master;
   MockConf conf(master.GetConf());
   master.UpdateCachedMessages(conf);
   protoMsg::ConfType configTypeMessage;
   std::string serializedConf;
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_SYSLOG);
   EXPECT_FALSE(serializedConf.empty());
   protoMsg::SyslogConf sConf;
   EXPECT_TRUE(sConf.ParseFromString(serializedConf));
   EXPECT_EQ("", sConf.siemlogging());
   sConf.set_siemlogging("false");
   conf.updateFields(sConf);
   master.UpdateCachedMessages(conf);
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_EQ("false", sConf.siemlogging());
#endif
}

TEST_F(ConfProcessorTests, TestSerializeCachedConfig) {
#ifdef LR_DEBUG
   MockConfMaster master;
   MockConf conf(master.GetConf());
   master.UpdateCachedMessages(conf);
   protoMsg::ConfType configTypeMessage;
   std::string serializedConf;

   configTypeMessage.set_direction(protoMsg::ConfType_Direction_RECEIVING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_RESTART);
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_TRUE(serializedConf.empty());
   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);
   serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   protoMsg::BaseConf baseConf;
   EXPECT_TRUE(baseConf.ParseFromString(serializedConf));

   configTypeMessage.set_type(protoMsg::ConfType_Type_QOSMOS);
   EXPECT_FALSE(serializedConf.empty());
   protoMsg::QosmosConf qConf;
   EXPECT_TRUE(qConf.ParseFromString(serializedConf));

   configTypeMessage.set_type(protoMsg::ConfType_Type_SHUTDOWN);
   EXPECT_FALSE(serializedConf.empty());
   protoMsg::ShutdownMsg shutdownConf;
   EXPECT_TRUE(shutdownConf.ParseFromString(serializedConf));


   configTypeMessage.set_type(protoMsg::ConfType_Type_SYSLOG);
   EXPECT_FALSE(serializedConf.empty());
   protoMsg::SyslogConf sConf;
   EXPECT_TRUE(sConf.ParseFromString(serializedConf));

   configTypeMessage.set_type(protoMsg::ConfType_Type_APP_VERSION);
   EXPECT_FALSE(serializedConf.empty());
   protoMsg::VersionMsg vConf;
   EXPECT_TRUE(vConf.ParseFromString(serializedConf));

#endif
}

TEST_F(ConfProcessorTests, TestReconcileNewConf) {
#ifdef LR_DEBUG
   MockConfMaster master;
   MockConf conf(master.GetConf());
   protoMsg::ConfType configTypeMessage;
   master.UpdateCachedMessages(conf);
   std::string serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   std::string message = master.SerializeCachedConfig(configTypeMessage);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);

   EXPECT_FALSE(master.ReconcileNewConf(configTypeMessage, conf, message));
   master.Start();

   master.SetConfLocation("/tmp/dummyWriteLocationCommandProcessorTests");
   MockConf conf2(master.GetConf());
   protoMsg::BaseConf baseConf;
   EXPECT_TRUE(baseConf.ParseFromString(serializedConf));
   conf2.updateFields(baseConf);

   EXPECT_TRUE(master.ReconcileNewConf(configTypeMessage, conf2, message));
   master.Stop();
   EXPECT_FALSE(master.ReconcileNewConf(configTypeMessage, conf2, message));
   master.Start();
   EXPECT_TRUE(master.ReconcileNewConf(configTypeMessage, conf2, message));
   configTypeMessage.set_type(::protoMsg::ConfType_Type_RESTART);
   EXPECT_FALSE(master.ReconcileNewConf(configTypeMessage, conf2, message));
   configTypeMessage.set_type(::protoMsg::ConfType_Type_APP_VERSION);
   EXPECT_FALSE(master.ReconcileNewConf(configTypeMessage, conf2, message));
   message = "abc123";
   EXPECT_FALSE(master.ReconcileNewConf(configTypeMessage, conf2, message));

   master.Stop();
   unlink(master.mConfLocation.c_str());
#endif
}

TEST_F(ConfProcessorTests, TestReconcileNewInterfaceConf) {
#ifdef LR_DEBUG
   MockConfMaster master;
   ConfNetInterface conf(mTestInterfaceConf);
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_NETINTERFACE);
   master.UpdateCachedMessage(conf);

   std::string serializedConf = master.SerializeCachedConfig(configTypeMessage);
   EXPECT_FALSE(serializedConf.empty());
   std::string message = master.SerializeCachedConfig(configTypeMessage);

   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf, message));
   master.Start();

   ConfNetInterface conf2(mWriteLocation);
   protoMsg::NetInterface interfaceMsg;
   EXPECT_TRUE(interfaceMsg.ParseFromString(serializedConf));
   conf2.updateFields(interfaceMsg);

   EXPECT_TRUE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   master.Stop();
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   master.Start();
   EXPECT_TRUE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   configTypeMessage.set_type(::protoMsg::ConfType_Type_RESTART);
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   configTypeMessage.set_type(::protoMsg::ConfType_Type_APP_VERSION);
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));
   message = "abc123";
   EXPECT_FALSE(master.ReconcileNewAbstractConf(configTypeMessage, conf2, message));

   master.Stop();
#endif
}

TEST_F(ConfProcessorTests, RestartMessagePassedBetweenMasterAndSlave) {
#if defined(LR_DEBUG)
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf conf(confThread.GetConf());
   MockConfSlave testSlave;
   testSlave.mBroadcastQueueName = conf.GetBroadcastQueue();
   ASSERT_FALSE(testSlave.mBroadcastQueueName.empty());
   testSlave.Start();
   sleep(1);


   ASSERT_FALSE(testSlave.mAppClosed);
   protoMsg::ConfType updateType;
   updateType.set_type(protoMsg::ConfType_Type_RESTART);
   updateType.set_direction(protoMsg::ConfType_Direction_SENDING);
   protoMsg::RestartMsg restartMsg;
   restartMsg.set_restartall(true);
   std::vector<std::string> encodedMessage;
   encodedMessage.push_back(updateType.SerializeAsString());
   encodedMessage.push_back(restartMsg.SerializeAsString());
   Crowbar confSender(conf.GetConfChangeQueue());
   ASSERT_TRUE(confSender.Wield());
   ASSERT_TRUE(confSender.Flurry(encodedMessage));
   ASSERT_TRUE(confSender.BlockForKill(encodedMessage));
   int sleepCount = 1;
   while (!testSlave.mAppClosed && sleepCount <= 20) {
      sleep(1);
      sleepCount++;
   }
   EXPECT_TRUE(testSlave.mAppClosed);
   testSlave.Stop();
   confThread.Stop();
#endif
}

TEST_F(ConfProcessorTests, DISABLED_ConfMessagePassedBetweenMasterAndSlave) {
#if defined(LR_DEBUG)
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   Conf conf(confThread.GetConf());
   MockConfSlave testSlave;
   testSlave.mBroadcastQueueName = conf.GetBroadcastQueue();
   ASSERT_FALSE(testSlave.mBroadcastQueueName.empty());
   testSlave.Start();
   sleep(1);

   EXPECT_FALSE(testSlave.mNewConfSeen);
   protoMsg::ConfType updateType;
   updateType.set_type(protoMsg::ConfType_Type_BASE);
   updateType.set_direction(protoMsg::ConfType_Direction_SENDING);
   protoMsg::BaseConf confMsg;
   std::vector<std::string> encodedMessage;

   encodedMessage.push_back(updateType.SerializeAsString());
   encodedMessage.push_back(confMsg.SerializeAsString());
   Crowbar confSender(conf.GetConfChangeQueue());
   EXPECT_TRUE(confSender.Wield());
   EXPECT_TRUE(confSender.Flurry(encodedMessage));
   EXPECT_TRUE(confSender.BlockForKill(encodedMessage));
   EXPECT_EQ(2, encodedMessage.size());
   EXPECT_FALSE(testSlave.mNewConfSeen);
   confMsg.ParseFromString(encodedMessage[1]);
   confMsg.set_pcapbuffersize("123");
   encodedMessage[1] = confMsg.SerializeAsString();
   EXPECT_TRUE(confSender.Flurry(encodedMessage));
   EXPECT_TRUE(confSender.BlockForKill(encodedMessage));
   int sleepCount = 1;
   while (!testSlave.mNewConfSeen && sleepCount <= 20) {
      sleep(1);
      sleepCount++;
   }
   EXPECT_TRUE(testSlave.mNewConfSeen);
   testSlave.Stop();
   confThread.Stop();
#endif
}

TEST_F(ConfProcessorTests, DISABLED_SyslogMessagePassedBetweenMasterAndSlave) {
#if defined(LR_DEBUG)
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   Conf conf(confThread.GetConf());
   MockConfSlave testSlave;
   testSlave.mBroadcastQueueName = conf.GetBroadcastQueue();
   ASSERT_FALSE(testSlave.mBroadcastQueueName.empty());
   testSlave.Start();
   sleep(1);

   ASSERT_FALSE(testSlave.mNewSyslogSeen);
   protoMsg::ConfType updateType;
   updateType.set_type(protoMsg::ConfType_Type_SYSLOG);
   updateType.set_direction(protoMsg::ConfType_Direction_SENDING);
   protoMsg::SyslogConf confMsg;
   std::vector<std::string> encodedMessage;

   encodedMessage.push_back(updateType.SerializeAsString());
   encodedMessage.push_back(confMsg.SerializeAsString());
   Crowbar confSender(conf.GetConfChangeQueue());
   ASSERT_TRUE(confSender.Wield());
   ASSERT_TRUE(confSender.Flurry(encodedMessage));
   ASSERT_TRUE(confSender.BlockForKill(encodedMessage));
   ASSERT_EQ(2, encodedMessage.size());
   ASSERT_FALSE(testSlave.mNewSyslogSeen);
   confMsg.ParseFromString(encodedMessage[1]);
   confMsg.set_syslogmaxlinelength("123");
   encodedMessage[1] = confMsg.SerializeAsString();
   ASSERT_TRUE(confSender.Flurry(encodedMessage));
   ASSERT_TRUE(confSender.BlockForKill(encodedMessage));
   sleep(3);
   int sleepCount = 1;
   while (!testSlave.mNewSyslogSeen && sleepCount <= 20) {
      sleep(1);
      sleepCount++;
   }
   EXPECT_TRUE(testSlave.mNewSyslogSeen);
   testSlave.Stop();
   confThread.Stop();
#endif
}

TEST_F(ConfProcessorTests, DISABLED_NetInterfaceMessagePassedBetweenMasterAndSlave) {
#if defined(LR_DEBUG)
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.Stop();
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   Conf conf(confThread.GetConf());
   MockConfSlave testSlave;
   testSlave.mBroadcastQueueName = conf.GetBroadcastQueue();
   ASSERT_FALSE(testSlave.mBroadcastQueueName.empty());
   testSlave.Start();
   sleep(1);

   EXPECT_FALSE(testSlave.mNewNetInterfaceMsg);
   protoMsg::ConfType updateType;
   updateType.set_type(protoMsg::ConfType_Type_NETINTERFACE);
   updateType.set_direction(protoMsg::ConfType_Direction_SENDING);
   protoMsg::NetInterface confMsg;
   confMsg.set_method(protoMsg::NetInterfaceMethod::STATICIP);
   confMsg.set_name("WOO");
   confMsg.set_ipaddress("24.24.24.24");
   confMsg.set_dnsservers("servers");
   confMsg.set_gateway("24.24.24.1");
   confMsg.set_netmask("255.255.255.0");
   std::vector<std::string> encodedMessage;

   encodedMessage.push_back(updateType.SerializeAsString());
   encodedMessage.push_back(confMsg.SerializeAsString());
   Crowbar confSender(conf.GetConfChangeQueue());
   EXPECT_TRUE(confSender.Wield());
   EXPECT_TRUE(confSender.Flurry(encodedMessage));
   EXPECT_TRUE(confSender.BlockForKill(encodedMessage));
   EXPECT_EQ(2, encodedMessage.size());
   int sleepCount = 1;
   while (!testSlave.mNewNetInterfaceMsg && sleepCount <= 20) {
      sleep(1);
      sleepCount++;
   }
   EXPECT_TRUE(testSlave.mNewNetInterfaceMsg);
   testSlave.Stop();
   confThread.Stop();
#endif
}

//Commenting out this test because it fails... essentially we can never update
//the map because it's never populated in the test.
//TEST_F(ConfProcessorTests, QosmosMessagePassedBetweenMasterAndSlave) {
//#if defined(LR_DEBUG)
//   ConfMaster& confThread = ConfMaster::Instance();
//   confThread.SetPath(mWriteLocation);
//   confThread.Start();
//   Conf conf(confThread.GetConf());
//   MockConfSlave testSlave;
//   testSlave.mBroadcastQueueName = conf.getBroadcastQueue();
//   testSlave.Start();
//   sleep(1);
//
//   ASSERT_FALSE(testSlave.mNewQosmosSeen);
//   protoMsg::ConfType updateType;
//   updateType.set_type(protoMsg::ConfType_Type_QOSMOS);
//   updateType.set_direction(protoMsg::ConfType_Direction_SENDING);
//   protoMsg::QosmosConf confMsg;
//   std::vector<std::string> encodedMessage;
//
//   encodedMessage.push_back(updateType.SerializeAsString());
//   encodedMessage.push_back(confMsg.SerializeAsString());
//   Crowbar confSender(conf.getConfChangeQueue());
//   ASSERT_TRUE(confSender.Wield());
//   ASSERT_TRUE(confSender.Flurry(encodedMessage));
//   ASSERT_TRUE(confSender.BlockForKill(encodedMessage));
//   ASSERT_EQ(2, encodedMessage.size());
//   ASSERT_FALSE(testSlave.mNewQosmosSeen);
//   confMsg.ParseFromString(encodedMessage[1]);
//
//   protoMsg::QosmosConf_Protocol * protocolToChange(confMsg.add_qosmosprotocol());
//   protocolToChange->set_protocolenabled(false);
//   protocolToChange->set_protocolfamily("foo");
//   protocolToChange->set_protocollongname("bar");
//   protocolToChange->set_protocolname("fubar");
//   encodedMessage[1] = confMsg.SerializeAsString();
//   ASSERT_FALSE(encodedMessage[1].empty());
//   LOG(DEBUG) << "Sending Qosmos conf";
//   ASSERT_TRUE(confSender.Flurry(encodedMessage));
//   ASSERT_TRUE(confSender.BlockForKill(encodedMessage));
//   int sleepCount(0);
//   while (!testSlave.mNewQosmosSeen && sleepCount <= 20) {
//      sleep(1);
//      sleepCount++;
//   }
//   LOG(DEBUG) << "Sent Qosmos conf";
//   EXPECT_TRUE(testSlave.mNewQosmosSeen);
//   testSlave.Stop();
//   confThread.Stop();
//#endif
//}

TEST_F(ConfProcessorTests, ValidateBasicMessageSize) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   protoMsg::BaseConf baseConfig;
   std::vector<std::string> shots;
   ASSERT_FALSE(testSlave.MessageHasPayload(shots));
   baseConfig.set_qosmosipdefragmentationenabled("false");
   shots.push_back(configTypeMessage.SerializeAsString());
   ASSERT_FALSE(testSlave.MessageHasPayload(shots));
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_TRUE(testSlave.MessageHasPayload(shots));
}
#ifdef LR_DEBUG
TEST_F(ConfProcessorTests, ProcessBaseMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);
   ASSERT_TRUE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
}
TEST_F(ConfProcessorTests, ProcessQosmosMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_QOSMOS);
   ASSERT_TRUE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
}
TEST_F(ConfProcessorTests, ProcessSyslogMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_SYSLOG);
   ASSERT_TRUE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
}
TEST_F(ConfProcessorTests, ProcessNetInterfaceMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_NETINTERFACE);
   ASSERT_TRUE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
}
TEST_F(ConfProcessorTests, ProcessNTPMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_NTP);
   ASSERT_TRUE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
}
TEST_F(ConfProcessorTests, ProcessRestartMsg_MoreToDo) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_RESTART);
   ASSERT_FALSE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
}
TEST_F(ConfProcessorTests, ProcessShutdownMsg_MoreToDo) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);
   configTypeMessage.set_type(protoMsg::ConfType_Type_SHUTDOWN);
   ASSERT_FALSE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
}
TEST_F(ConfProcessorTests, ProcessRestartMsg) {

   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   protoMsg::RestartMsg baseConfig;
   std::vector<std::string> shots;
   MockConf conf;
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());

   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_FALSE(testSlave.ProcessRestartMsg(configTypeMessage, shots));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   configTypeMessage.set_type(protoMsg::ConfType_Type_APP_VERSION);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_FALSE(testSlave.ProcessRestartMsg(configTypeMessage, shots));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   configTypeMessage.set_type(protoMsg::ConfType_Type_RESTART);
   ASSERT_FALSE(testSlave.MessageRequiresNoFurtherAction(configTypeMessage.type()));
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_FALSE(testSlave.ProcessRestartMsg(configTypeMessage, shots));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   baseConfig.set_restartall(false);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_TRUE(testSlave.ProcessRestartMsg(configTypeMessage, shots));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   baseConfig.set_restartall(true);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_TRUE(testSlave.ProcessRestartMsg(configTypeMessage, shots));
   ASSERT_TRUE(testSlave.mAppClosed);

}
#endif

TEST_F(ConfProcessorTests, testSingletonInstantiation) {
   ConfMaster& confMaster = ConfMaster::Instance();
   ConfMaster& confMaster2 = ConfMaster::Instance();
   EXPECT_EQ(&confMaster,&confMaster2);
   //   confThread.Stop();
}

//TEST_F(ConfProcessorTests, testNewConfConstructionStream) {
//   std::stringstream stream;
//   std::stringstream qStream;
//   std::stringstream sStream;
//   Conf * conf = new Conf(stream, qStream, sStream);
//   delete conf;
//}

TEST_F(ConfProcessorTests, testNewConfConstructionUpdateMsg) {
   protoMsg::BaseConf msg;
   QosmosConf qmsg;
   protoMsg::SyslogConf sysMsg;
   MockConf * conf = new MockConf(msg, qmsg, sysMsg);
   delete conf;
}

TEST_F(ConfProcessorTests, getScrubPasswordEnabled) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   //get empty conf
   MockConf conf = confThread.GetConf();
   EXPECT_TRUE(conf.GetScrubPasswordsEnabled());

}

TEST_F(ConfProcessorTests, testConfIntDefaults) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.Start();
   //get empty conf
   confThread.SetPath(mWriteLocation);
   Conf conf = confThread.GetConf();
   int packetRecv = conf.GetPacketRecvQueueSize();
   int packetSend = conf.GetPacketSendQueueSize();
   int pcapBSize = conf.GetPCAPBuffsize();
   int dpiSend = conf.GetDPIMsgSendQueueSize();
   int dpiRecv = conf.GetDPIMsgRecvQueueSize();
   int syslogSend = conf.GetSyslogSendQueueSize();
   int syslogRecv = conf.GetSyslogRecvQueueSize();
   //Expect the defaults that are in the #define
   EXPECT_EQ(PACKET_SEND_QUEUE_SIZE, packetSend);
   EXPECT_EQ(PACKET_RECV_QUEUE_SIZE, packetRecv);
   EXPECT_EQ(PCAP_BUFFER_SIZE, pcapBSize);
   EXPECT_EQ(DPI_MSG_SEND_QUEUE_SIZE, dpiSend);
   EXPECT_EQ(DPI_MSG_RECV_QUEUE_SIZE, dpiRecv);
   EXPECT_EQ(SYSLOG_SEND_QUEUE_SIZE, syslogSend);
   EXPECT_EQ(SYSLOG_RECV_QUEUE_SIZE, syslogRecv);
   EXPECT_EQ(MAX_SYSLOG_LINE_RFC_5426, conf.GetSyslogMaxLineLength());
   EXPECT_FALSE(conf.GetQosmosDebugModeEnabled());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS, conf.GetDpiHalfSessions());
   EXPECT_TRUE(conf.GetEnableIPDefragmentation());
   EXPECT_TRUE(conf.GetEnableTCPReassembly());
   EXPECT_FALSE(conf.GetSiemLogging());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS * 5, conf.GetQosmos64BytePool());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS * 2.5, conf.GetQosmos128BytePool());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS, conf.GetQosmos256BytePool());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS * .75, conf.GetQosmos512BytePool());
   EXPECT_EQ(DEFAULT_SESSION_EXPIRE_PER_PROCESS, conf.GetQosmosExpirePerCallback());
   EXPECT_FALSE(conf.GetSiemDebugLogging());
   confThread.Stop();
}

TEST_F(ConfProcessorTests, testGetConfFromFile) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   //runs from test/ directory.
   //   Conf conf = confThread.GetConf();
   Conf conf = confThread.GetConf(mTestConf);
   EXPECT_EQ("/etc/rsyslog.d/nm.rsyslog.conf", conf.GetSyslogConfigFile());
   EXPECT_EQ(true, conf.GetSyslogTcpEnabled());
   EXPECT_EQ("local4", conf.GetSyslogFacility());
   EXPECT_EQ("10.1.1.67", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.GetDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/syslogQ.ipc", conf.GetSyslogQueue());
   EXPECT_EQ("ipc:///tmp/statsAccumulatorQ.ipc", conf.GetStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/sendStatsQ.ipc", conf.GetSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.GetConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.GetCommandQueue());
   EXPECT_EQ("/usr/local/nm/logs", conf.GetLogDir());
   EXPECT_TRUE(8 == conf.GetDpiThreads());
   EXPECT_EQ(30, conf.GetPCAPETimeOut());
   EXPECT_EQ(13, conf.GetPCAPBuffsize());
   //   EXPECT_EQ("eth0", conf.getPCAPInterface()); this is now internally validated
   EXPECT_TRUE(conf.GetSyslogEnabled());
   EXPECT_EQ(2047, conf.GetSyslogMaxLineLength());
   EXPECT_EQ(250000, conf.GetDpiHalfSessions());
   EXPECT_FALSE(conf.GetEnableIPDefragmentation());
   EXPECT_FALSE(conf.GetEnableTCPReassembly());
   EXPECT_EQ(50, conf.GetPacketRecvQueueSize());
   EXPECT_EQ(100, conf.GetPacketSendQueueSize());
   EXPECT_EQ(15000, conf.GetDPIMsgRecvQueueSize());
   EXPECT_EQ(30000, conf.GetDPIMsgSendQueueSize());
   EXPECT_EQ(800, conf.GetSyslogRecvQueueSize());
   EXPECT_EQ(500, conf.GetSyslogSendQueueSize());
   EXPECT_EQ(1, conf.GetStatsIntervalSeconds());
   EXPECT_TRUE(conf.GetQosmosDebugModeEnabled());
   EXPECT_TRUE(conf.GetSiemLogging());
   EXPECT_TRUE(conf.GetSiemDebugLogging());
   EXPECT_EQ(64, conf.GetQosmos64BytePool());
   EXPECT_EQ(128, conf.GetQosmos128BytePool());
   EXPECT_EQ(256, conf.GetQosmos256BytePool());
   EXPECT_EQ(512, conf.GetQosmos512BytePool());
   EXPECT_EQ(123, conf.GetQosmosExpirePerCallback());
   confThread.Stop();

}

TEST_F(ConfProcessorTests, testGetConfFromString) {
   ConfMaster& confThread = ConfMaster::Instance();
   //runs from test/ directory.
   Conf conf = confThread.GetConf(mTestConf);
   EXPECT_EQ("/etc/rsyslog.d/nm.rsyslog.conf", conf.GetSyslogConfigFile());
   EXPECT_EQ(true, conf.GetSyslogTcpEnabled());
   EXPECT_EQ("local4", conf.GetSyslogFacility());
   EXPECT_EQ("10.1.1.67", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.GetDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/syslogQ.ipc", conf.GetSyslogQueue());
   EXPECT_EQ("ipc:///tmp/statsAccumulatorQ.ipc", conf.GetStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/sendStatsQ.ipc", conf.GetSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.GetConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.GetCommandQueue());
   EXPECT_EQ("/usr/local/nm/logs", conf.GetLogDir());
   EXPECT_TRUE(8 == conf.GetDpiThreads());
   EXPECT_EQ(30, conf.GetPCAPETimeOut());
   EXPECT_EQ(13, conf.GetPCAPBuffsize());
   //   EXPECT_EQ("eth0", conf.getPCAPInterface());  this is now internally validated
   EXPECT_TRUE(conf.GetSyslogEnabled());
   EXPECT_EQ(2047, conf.GetSyslogMaxLineLength());
   EXPECT_EQ(1, conf.GetStatsIntervalSeconds());
}

TEST_F(ConfProcessorTests, testGetConfInvalidFile) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   //runs from test/ directory.
   Conf conf = confThread.GetConf();
   EXPECT_EQ("/etc/rsyslog.d/nm.rsyslog.conf", conf.GetSyslogConfigFile()); // default value
   EXPECT_EQ("", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort()); // default value
   EXPECT_FALSE(conf.GetSyslogTcpEnabled()); // default value
   EXPECT_EQ("local4", conf.GetSyslogFacility()); // default value
   
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.GetDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/syslogQ.ipc", conf.GetSyslogQueue());
   EXPECT_EQ("ipc:///tmp/broadcast.ipc", conf.GetBroadcastQueue());
   EXPECT_EQ("ipc:///tmp/statsAccumulatorQ.ipc", conf.GetStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/statsmsg.ipc", conf.GetSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.GetConfChangeQueue());
   EXPECT_EQ("tcp://127.0.0.1:5556", conf.GetCommandQueue());
   std::string expectedDir = INSTALL_PREFIX;
   expectedDir += "/logs";
   EXPECT_EQ(expectedDir, conf.GetLogDir());
   EXPECT_EQ(NUMBER_OF_QOSMOS_THREADS, conf.GetDpiThreads());
   EXPECT_EQ(PCAP_ETIMEDOUT, conf.GetPCAPETimeOut());
   EXPECT_EQ(PCAP_BUFFER_SIZE, conf.GetPCAPBuffsize());
   //   EXPECT_EQ("", conf.getPCAPInterface());  internally validated
   EXPECT_TRUE(conf.GetSyslogEnabled());
   EXPECT_EQ(MAX_SYSLOG_LINE_RFC_5426, conf.GetSyslogMaxLineLength());
   EXPECT_EQ(DEFAULT_STATS_INTERVAL_SEC, conf.GetStatsIntervalSeconds());
   EXPECT_EQ("scripts", conf.GetScriptsDir());
   confThread.Stop();

}

TEST_F(ConfProcessorTests, testConfSyslogEnabled) {
   protoMsg::SyslogConf msg;
   std::string expSyslogEnabled("false");
   msg.set_syslogenabled(expSyslogEnabled);
   MockConf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.GetSyslogEnabled());
}

TEST_F(ConfProcessorTests, testConfSyslogSpecified) {
   protoMsg::SyslogConf msg;
   msg.set_syslogenabled("true");
   msg.set_sysloglogagentip("123.123.123");
   msg.set_sysloglogagentport("777");
   msg.set_syslogtcpenabled("false");
   EXPECT_EQ("false", msg.syslogtcpenabled());
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_TRUE(conf.GetSyslogEnabled());
   EXPECT_FALSE(conf.GetSyslogTcpEnabled());


   EXPECT_EQ("123.123.123", conf.GetSyslogAgentIP());
   EXPECT_EQ("777", conf.GetSyslogAgentPort());
   EXPECT_EQ("local4", conf.GetSyslogFacility());
   EXPECT_EQ("/etc/rsyslog.d/nm.rsyslog.conf", conf.GetSyslogConfigFile());
   
   
   msg.set_syslogtcpenabled("true");
   conf.updateFields(msg);
   EXPECT_TRUE(conf.GetSyslogTcpEnabled());
   
}

TEST_F(ConfProcessorTests, testConfSyslogDefaults) {
   protoMsg::SyslogConf msg;

   MockConfExposeUpdate conf("non-existent-yaml-file");
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_TRUE(conf.GetSyslogEnabled());
   EXPECT_EQ("", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());
   EXPECT_EQ("local4", conf.GetSyslogFacility());
   EXPECT_EQ("/etc/rsyslog.d/nm.rsyslog.conf", conf.GetSyslogConfigFile());
   EXPECT_FALSE(conf.GetSyslogTcpEnabled());  // default is UDP

   EXPECT_TRUE(conf.GetScrubPasswordsEnabled());
}




TEST_F(ConfProcessorTests, testConfSyslogDisabled) {
   protoMsg::SyslogConf msg;
   std::string expSyslogEnabled("false");
   msg.set_syslogenabled(expSyslogEnabled);
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.GetSyslogEnabled());
}

TEST_F(ConfProcessorTests, testConfQosmosDebugModeEnabled) {
   protoMsg::BaseConf msg;
   std::string qosmosDebugModeEnabled("true");
   msg.set_qosmosdebugmodeenabled(qosmosDebugModeEnabled);
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_TRUE(conf.GetQosmosDebugModeEnabled());
}

TEST_F(ConfProcessorTests, testConfQosmos512BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos512bytepool("1234"); // Range{500000 , 8000000}
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_NE(1234, conf.GetQosmos512BytePool());

   msg.set_qosmos512bytepool("500001");
   conf.updateFields(msg);
   EXPECT_EQ(500001, conf.GetQosmos512BytePool());
}

TEST_F(ConfProcessorTests, testConfQosmos256BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos256bytepool("8000000"); // Range{500000, 8000000}
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(8000000, conf.GetQosmos256BytePool());
}

TEST_F(ConfProcessorTests, testConfQosmos128BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos128bytepool("3000000"); // Range{3000000, 8000000}
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(3000000, conf.GetQosmos128BytePool());
}

TEST_F(ConfProcessorTests, testConfQosmos64BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos64bytepool("1500000"); // Range{1500000,8000000}
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(1500000, conf.GetQosmos64BytePool());
}

TEST_F(ConfProcessorTests, testQosmosExpirePerCallback) {
   protoMsg::BaseConf msg;
   msg.set_qosmosexpirepercallback("100"); // Range{1,100}
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(100, conf.GetQosmosExpirePerCallback());
}

TEST_F(ConfProcessorTests, testQosmosTCPReAssembly) {
   protoMsg::BaseConf msg;
   msg.set_qosmostcpreassemblyenabled("false");
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.GetEnableTCPReassembly());
}

TEST_F(ConfProcessorTests, testEnableIPDefragmentation) {
   protoMsg::BaseConf msg;
   msg.set_qosmosipdefragmentationenabled("false");
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.GetEnableIPDefragmentation());
}

TEST_F(ConfProcessorTests, teststatsAccumulatorQueue) {
   protoMsg::BaseConf msg;
   msg.set_statsaccumulatorqueue("123456");// at least 6 characters
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ("123456", conf.GetStatsAccumulatorQueue());
}

TEST_F(ConfProcessorTests, teststatsQueue) {
   protoMsg::BaseConf msg;
   msg.set_sendstatsqueue("123456");
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ("123456", conf.GetSendStatsQueue());
}

TEST_F(ConfProcessorTests, testSiemLogging) {
   protoMsg::SyslogConf msg;
   msg.set_siemlogging("false");
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.GetSiemLogging());
   msg.set_siemlogging("true");
   conf.updateFields(msg);
   EXPECT_TRUE(conf.GetSiemLogging());

}

TEST_F(ConfProcessorTests, testCommandQueue) {
   protoMsg::BaseConf msg;
   msg.set_commandqueue("this is false"); // minimum 6 characters
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ("this is false", conf.GetCommandQueue());
   msg.set_commandqueue("now it is true");
   conf.updateFields(msg);
   EXPECT_EQ("now it is true", conf.GetCommandQueue());
}

TEST_F(ConfProcessorTests, testSiemDebugLogging) {
   protoMsg::SyslogConf msg;
   msg.set_debugsiemlogging("false");
   MockConfExposeUpdate conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.GetSiemDebugLogging());
   msg.set_debugsiemlogging("true");
   conf.updateFields(msg);
   EXPECT_TRUE(conf.GetSiemDebugLogging());

}

TEST_F(ConfProcessorTests, testProtoMessage) {
   protoMsg::BaseConf msg;
   protoMsg::SyslogConf sysMsg;
   std::string expAgentIP = "24.24.24.24";
   std::string expAgentPort = "514";
   std::string expLogFacility = "local4";
   std::string dpiThreads = "13";
   std::string pcapTimeout = "30";
   std::string pcapBufferSize = "30";
   std::string pcapInterface = "foo";
   std::string syslogMaxLineLength = "1234";
   sysMsg.set_sysloglogagentip(expAgentIP);
   sysMsg.set_sysloglogagentport(expAgentPort);
   sysMsg.set_debugsiemlogging("true");
   msg.set_dpithreads(dpiThreads);
   msg.set_pcapetimeout(pcapTimeout);
   msg.set_pcapbuffersize(pcapBufferSize);
   msg.set_pcapinterface(pcapInterface);
   sysMsg.set_syslogmaxlinelength(syslogMaxLineLength);
   QosmosConf qmsg;
   MockConfExposeUpdate conf(msg, qmsg, sysMsg);

   EXPECT_EQ(expAgentIP, conf.GetSyslogAgentIP());
   EXPECT_EQ(expAgentPort, conf.GetSyslogAgentPort());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(dpiThreads), conf.GetDpiThreads());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(pcapTimeout), conf.GetPCAPETimeOut());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(pcapBufferSize), conf.GetPCAPBuffsize());
   EXPECT_EQ(pcapInterface, conf.GetPCAPInterface());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(syslogMaxLineLength), conf.GetSyslogMaxLineLength());
   EXPECT_TRUE(conf.GetSiemDebugLogging());
}

TEST_F(ConfProcessorTests, testIpOnlyProtoMessage) {
   protoMsg::SyslogConf sysMsg;
   protoMsg::BaseConf msg;
   std::string expAgentIP = "24.24.24.24";
   sysMsg.set_sysloglogagentip(expAgentIP);
   QosmosConf qmsg;
   MockConfExposeUpdate conf(msg, qmsg, sysMsg);
   EXPECT_EQ(expAgentIP, conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort()); // default
}

//TEST_F(ConfProcessorTests, testWriteToFile) {
//   protoMsg::SyslogConf sysMsg;
//   protoMsg::BaseConf msg;
//   std::string expAgentIP = "24.24.24.24";
//   sysMsg.set_sysloglogagentip(expAgentIP);
//   QosmosConf qmsg;
//   Conf conf(msg, qmsg, sysMsg);
//   std::stringstream stream;
//   conf.writeToStream(stream);
//   std::stringstream qStream;
//   conf.writeQosmosToStream(qStream);
//   std::stringstream sStream;
//   conf.writeSyslogToStream(sStream);
//
//   Conf newConf(stream, qStream, sStream);
//
//   //TODO:: add compare operator so that we can do EXPECT_EQ(conf, newConf);
//   EXPECT_EQ(conf.getSyslogAgentIP(), newConf.getSyslogAgentIP());
//}

TEST_F(ConfProcessorTests, testRealChangeAndWriteToDisk) {
   //runs from test/ directory.
   MockConfExposeUpdate conf(mTestConf);
   EXPECT_EQ("10.1.1.67", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());

   conf.setPath(mWriteLocation);
   ASSERT_EQ(mWriteLocation, conf.GetPath());
   protoMsg::BaseConf msg;
   protoMsg::SyslogConf sysMsg;
   std::string expAgentIP = "24.24.24.24";

   sysMsg.set_sysloglogagentip(expAgentIP);
   conf.updateFields(sysMsg);

   EXPECT_EQ(expAgentIP, conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());

   conf.writeSyslogToFile();

   MockConfExposeUpdate newConf(mWriteLocation);

   EXPECT_EQ(expAgentIP, newConf.GetSyslogAgentIP());
   EXPECT_EQ("514", newConf.GetSyslogAgentPort());

}

TEST_F(ConfProcessorTests, testPathWithDynamicConf) {
   MockConfExposeUpdate * conf = new MockConfExposeUpdate("/path");
   std::string expPath = "/tmp/new_path";
   conf->setPath(expPath);
   EXPECT_EQ(expPath, conf->GetPath());
   delete conf;
}

TEST_F(ConfProcessorTests, testGetBaseConfMsg) {

   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf currentConf = confThread.GetConf();
   Crowbar confChangeQ(currentConf.GetConfChangeQueue());
   ASSERT_TRUE(confChangeQ.Wield());

   protoMsg::ConfType ctm;
   ctm.set_direction(protoMsg::ConfType_Direction_RECEIVING); // Receiving conf from ConfMaster
   ctm.set_type(protoMsg::ConfType_Type_BASE); // Request Base Conf Message
   std::string ctms = ctm.SerializeAsString();
   std::string msg(""); // Empty Conf Message
   std::vector<std::string> messages;
   messages.push_back(ctms);
   messages.push_back(msg);

   ASSERT_TRUE(confChangeQ.Flurry(messages));
   std::vector<std::string> data;
   EXPECT_TRUE(confChangeQ.BlockForKill(data));
   ASSERT_EQ(data.size(), 2);

   protoMsg::ConfType ctmRsp;
   ctmRsp.ParseFromString(data[0]);
   ASSERT_TRUE(ctmRsp.direction() == protoMsg::ConfType_Direction_SENDING);
   ASSERT_TRUE(ctmRsp.type() == protoMsg::ConfType_Type_BASE);

   MockConf conf;
   protoMsg::BaseConf confUpdateMsg;
   confUpdateMsg.ParseFromString(data[1]);
   conf.updateFields(confUpdateMsg);
   EXPECT_EQ(5, conf.GetStatsIntervalSeconds());

   confThread.Stop();
}

TEST_F(ConfProcessorTests, testGetSyslogConfMsg) {

   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf currentConf = confThread.GetConf();
   Crowbar confChangeQ(currentConf.GetConfChangeQueue());
   ASSERT_TRUE(confChangeQ.Wield());

   protoMsg::ConfType ctm;
   ctm.set_direction(protoMsg::ConfType_Direction_RECEIVING); // Receiving conf from ConfMaster
   ctm.set_type(protoMsg::ConfType_Type_SYSLOG); // Request Base Conf Message
   std::string ctms = ctm.SerializeAsString();
   std::string msg(""); // Empty Conf Message
   std::vector<std::string> messages;
   messages.push_back(ctms);
   messages.push_back(msg);

   ASSERT_TRUE(confChangeQ.Flurry(messages));
   std::vector<std::string> data;
   EXPECT_TRUE(confChangeQ.BlockForKill(data));
   ASSERT_EQ(data.size(), 2);

   protoMsg::ConfType ctmRsp;
   ctmRsp.ParseFromString(data[0]);
   ASSERT_TRUE(ctmRsp.direction() == protoMsg::ConfType_Direction_SENDING);
   ASSERT_TRUE(ctmRsp.type() == protoMsg::ConfType_Type_SYSLOG);

   MockConfExposeUpdate conf;
   protoMsg::SyslogConf confUpdateMsg;
   EXPECT_TRUE(confUpdateMsg.ParseFromString(data[1]));
   conf.updateFields(confUpdateMsg);
   EXPECT_EQ("10.1.1.67", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());

   confThread.Stop();
}

TEST_F(ConfProcessorTests, testSendGarbageZmqMsgs) {

   LOG(DEBUG) << "testSendGarbageZmqMsgs begin";
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf currentConf = confThread.GetConf();
   Crowbar confChangeQ(currentConf.GetConfChangeQueue());
   ASSERT_TRUE(confChangeQ.Wield());
   std::string msg("adasdfasdf");
   std::vector<std::string> messages;
   messages.push_back(msg);
   messages.push_back(msg);
   ASSERT_TRUE(confChangeQ.Flurry(messages));
   std::vector<std::string> data;
   // Empty response to garbage
   EXPECT_TRUE(confChangeQ.WaitForKill(data, 10000));
   confThread.Stop();
   ASSERT_EQ(2, data.size());
   EXPECT_TRUE(data[1].empty());
   LOG(DEBUG) << "testSendGarbageZmqMsgs end";
}

TEST_F(ConfProcessorTests, testPolledConsumerRcvNoReg) {
   ConfMaster& confThread = ConfMaster::Instance();
   Conf conf = confThread.GetConf();
   EXPECT_FALSE(confThread.ReceiveConf((void *) &conf, conf));
   confThread.Stop();
}

TEST_F(ConfProcessorTests, testPolledConsumerMultiRegNoReg) {
   ConfMaster& confThread = ConfMaster::Instance();
   Conf conf = confThread.GetConf();
   confThread.RegisterConsumer((void *) &conf);
   confThread.RegisterConsumer((void *) &conf);
   confThread.UnregisterConsumer((void *) &conf);
   confThread.UnregisterConsumer((void *) &conf);
   EXPECT_FALSE(confThread.ReceiveConf((void *) &conf, conf));
   confThread.Stop();
}

TEST_F(ConfProcessorTests, testPolledConsumerRcvAfterReg) {

   ConfMaster& confThread = ConfMaster::Instance();
   //runs from test/ directory.
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf conf = confThread.GetConf();
   confThread.RegisterConsumer((void *) &conf);

   EXPECT_TRUE(confThread.ReceiveConf((void *) &conf, conf));

   EXPECT_EQ("10.1.1.67", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.GetDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/syslogQ.ipc", conf.GetSyslogQueue());
   EXPECT_EQ("ipc:///tmp/statsAccumulatorQ.ipc", conf.GetStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/sendStatsQ.ipc", conf.GetSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.GetConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.GetCommandQueue());
   EXPECT_TRUE(8 == conf.GetDpiThreads());
   EXPECT_EQ(1, conf.GetStatsIntervalSeconds());
   EXPECT_TRUE(conf.GetSyslogEnabled());
   EXPECT_EQ("../scripts", conf.GetScriptsDir());

   EXPECT_FALSE(confThread.ReceiveConf((void *) &conf, conf));
   confThread.UnregisterConsumer((void *) &conf);
   confThread.Stop();
}

TEST_F(ConfProcessorTests, testPolledConsumerRcvAfterUnreg) {
   ConfMaster& confThread = ConfMaster::Instance();
   //runs from test/ directory.
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   Conf conf = confThread.GetConf();
   confThread.RegisterConsumer((void *) &conf);
   confThread.UnregisterConsumer((void *) &conf);
   EXPECT_FALSE(confThread.ReceiveConf((void *) &conf, conf));
   confThread.Stop();
}

TEST_F(ConfProcessorTests, testPolledConsumerRcvAfterNotify) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   Conf conf = confThread.GetConf();
   confThread.RegisterConsumer((void *) &conf);

   Crowbar confChangeQ(conf.GetConfChangeQueue());
   //std::cout << conf.getConfChangeQueue() << " sender" << std::endl;
   ASSERT_TRUE(confChangeQ.Wield());

   protoMsg::ConfType ctm;
   ctm.set_direction(protoMsg::ConfType_Direction_SENDING); // Sending conf to ConfMaster
   ctm.set_type(protoMsg::ConfType_Type_SYSLOG); // Base Conf Message
   std::string ctms = ctm.SerializeAsString();
   std::vector<std::string> messages;
   messages.push_back(ctms);

   protoMsg::SyslogConf bcm;

   std::string expLogAgentIP("24.24.24.24");
   bcm.set_sysloglogagentip(expLogAgentIP);

   std::string bcms = bcm.SerializeAsString();
   messages.push_back(bcms);

   //std::cout << "Sending config" << std::endl;
   ASSERT_TRUE(confChangeQ.Flurry(messages));
   std::vector<std::string> reply;
   //std::cout << "waiting for config" << std::endl;
   EXPECT_TRUE(confChangeQ.BlockForKill(reply));

   EXPECT_TRUE(confThread.ReceiveConf((void *) &conf, conf));
   confThread.UnregisterConsumer((void *) &conf);
   confThread.Stop();
}

TEST_F(ConfProcessorTests, testConfSlaveBasic) {
   ConfSlave& slave = ConfSlave::Instance();
   slave.SetPath(mTestConf);
   slave.Start();
   Conf conf = slave.GetConf();
   EXPECT_EQ("10.1.1.67", conf.GetSyslogAgentIP());
   EXPECT_EQ("514", conf.GetSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.GetDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/syslogQ.ipc", conf.GetSyslogQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.GetConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.GetCommandQueue());
   EXPECT_EQ("/usr/local/nm/logs", conf.GetLogDir());
   EXPECT_EQ(8, conf.GetDpiThreads());
   EXPECT_EQ(30, conf.GetPCAPETimeOut());
   EXPECT_EQ(13, conf.GetPCAPBuffsize());
   //   EXPECT_EQ("eth0", conf.getPCAPInterface());  internally validated
   EXPECT_TRUE(conf.GetSyslogEnabled());
   EXPECT_EQ(2047, conf.GetSyslogMaxLineLength());
   slave.Stop();
}

TEST_F(ConfProcessorTests, testConfSlaveUpdate) {
   LOG(DEBUG) << "testConfSlaveUpdate start";
   MockConfMaster master;
   ConfSlave& slave = ConfSlave::Instance();
   master.Stop();
   slave.Stop();
   master.SetConfLocation(mWriteLocation);
   master.SetPath(mWriteLocation);
   slave.SetPath(mWriteLocation);
   master.Start();
   slave.Start();
   sleep(1);
   Conf masterConf = master.GetConf();
   Conf slaveConf = slave.GetConf();
   MockConfExposeUpdate normalConf = slave.GetConf(mTestConf);

   master.RegisterConsumer((void *) &masterConf);
   slave.RegisterConsumer((void *) &slaveConf);

   ASSERT_NE(&masterConf, &slaveConf);

   //validate that there is nothing new to receive.
   ASSERT_TRUE(master.ReceiveConf((void *) &masterConf, masterConf));
   ASSERT_TRUE(slave.ReceiveConf((void *) &slaveConf, slaveConf));

   EXPECT_FALSE(master.ReceiveConf((void *) &masterConf, masterConf));
   EXPECT_FALSE(slave.ReceiveConf((void *) &slaveConf, slaveConf));

   ASSERT_TRUE(normalConf.sendConfigUpdate());
   //expect all confs to be updated
   sleep(1);
   ASSERT_TRUE(master.ReceiveConf((void *) &masterConf, masterConf));
   int tries = 50;
   while (tries >= 0) {
      if (slave.ReceiveConf((void *) &slaveConf, slaveConf)) {
         break;
      } else {
         tries--;
         if (tries < 0) {
            //failed to receive conf.
            ASSERT_TRUE(false);
         }
         sleep(1);
      }
   }

   //test results equal normal conf
   EXPECT_EQ(normalConf.GetSyslogAgentIP(), slaveConf.GetSyslogAgentIP());
   EXPECT_EQ(normalConf.GetSyslogAgentPort(), slaveConf.GetSyslogAgentPort());
   EXPECT_EQ(normalConf.GetDpiRcvrQueue(), slaveConf.GetDpiRcvrQueue());
   EXPECT_EQ(normalConf.GetSyslogQueue(), slaveConf.GetSyslogQueue());
   EXPECT_EQ(normalConf.GetConfChangeQueue(), slaveConf.GetConfChangeQueue());
   EXPECT_EQ(normalConf.GetCommandQueue(), slaveConf.GetCommandQueue());
   EXPECT_EQ(normalConf.GetDpiThreads(), slaveConf.GetDpiThreads());
   EXPECT_EQ(normalConf.GetPCAPETimeOut(), slaveConf.GetPCAPETimeOut());
   EXPECT_EQ(normalConf.GetPCAPBuffsize(), slaveConf.GetPCAPBuffsize());
   EXPECT_EQ(normalConf.GetPCAPInterface(), slaveConf.GetPCAPInterface());
   EXPECT_EQ(normalConf.GetSyslogEnabled(), slaveConf.GetSyslogEnabled());

   EXPECT_EQ(normalConf.GetSyslogAgentIP(), masterConf.GetSyslogAgentIP());
   EXPECT_EQ(normalConf.GetSyslogAgentPort(), masterConf.GetSyslogAgentPort());
   EXPECT_EQ(normalConf.GetDpiRcvrQueue(), masterConf.GetDpiRcvrQueue());
   EXPECT_EQ(normalConf.GetSyslogQueue(), masterConf.GetSyslogQueue());
   EXPECT_EQ(normalConf.GetConfChangeQueue(), masterConf.GetConfChangeQueue());
   EXPECT_EQ(normalConf.GetCommandQueue(), masterConf.GetCommandQueue());
   EXPECT_EQ(normalConf.GetDpiThreads(), masterConf.GetDpiThreads());
   EXPECT_EQ(normalConf.GetPCAPETimeOut(), masterConf.GetPCAPETimeOut());
   EXPECT_EQ(normalConf.GetPCAPBuffsize(), masterConf.GetPCAPBuffsize());
   EXPECT_EQ(normalConf.GetPCAPInterface(), masterConf.GetPCAPInterface());
   EXPECT_EQ(normalConf.GetSyslogEnabled(), masterConf.GetSyslogEnabled());

   master.UnregisterConsumer((void *) &masterConf);
   slave.UnregisterConsumer((void *) &slaveConf);
   master.Stop();
   slave.Stop();
   LOG(DEBUG) << "testConfSlaveUpdate stop";
}

TEST_F(ConfProcessorTests, testConfMasterShutdown) {
   ConfMaster& master = ConfMaster::Instance();
   master.SetPath(mTestConf);
   Conf conf = master.GetConf();
   master.Stop();
   master.Stop();
   master.Start();
   master.Start();
   master.Stop();
   master.Stop();
}

TEST_F(ConfProcessorTests, testConfSlaveShutdown) {
   ConfSlave& slave = ConfSlave::Instance();
   slave.SetPath(mTestConf);
   Conf conf = slave.GetConf();
   slave.Stop();
   slave.Stop();
   slave.Start();
   slave.Start();
   slave.Stop();
   slave.Stop();
}

TEST_F(ConfProcessorTests, testSetandGetQosmosConfig) {
   MockConf conf("/tmp/path/that/doesnt/exist/woo.ls");
   QosmosConf qConf = conf.GetQosmosConfigInfo();
   ASSERT_EQ(0, qConf.qosmosprotocol_size());

   std::string fakeProto = "amazon";
   ProtocolFamilyMap familyInfo;
   ProtocolEnabledMap enabledInfo;
   familyInfo[fakeProto] = FamilyLongProtocolPair("Web", "amazon");
   enabledInfo[fakeProto] = false;
   fakeProto = "amazon_aws";
   familyInfo[fakeProto] = FamilyLongProtocolPair("Web", "Amazon Web Services");
   enabledInfo[fakeProto] = true;

   ASSERT_TRUE(qConf.ReconcileWithMaps(familyInfo, enabledInfo));
   ASSERT_EQ(2, qConf.qosmosprotocol_size());
   conf.updateQosmos(qConf);
   QosmosConf pConf = conf.GetQosmosConfigInfo();
   for (int i = 0; i < pConf.qosmosprotocol_size(); i++) {
      const protoMsg::QosmosConf::Protocol& existingProtocol = pConf.qosmosprotocol(i);

      std::string protocolName = existingProtocol.protocolname();
      ASSERT_EQ(familyInfo[protocolName].first, existingProtocol.protocolfamily());
      ASSERT_EQ(familyInfo[protocolName].second, existingProtocol.protocollongname());
      ASSERT_EQ(enabledInfo[protocolName], existingProtocol.protocolenabled());

   }
}

TEST_F(ConfProcessorTests, testWriteQosmosToFile) {
   MockConf conf("/tmp/path/that/doesnt/exist/woo.ls");
   QosmosConf qConf = conf.GetQosmosConfigInfo();
   ASSERT_EQ(0, qConf.qosmosprotocol_size());

   std::string fakeProto = "amazon";
   ProtocolFamilyMap familyInfo;
   ProtocolEnabledMap enabledInfo;
   familyInfo[fakeProto] = FamilyLongProtocolPair("Web", "amazon");
   enabledInfo[fakeProto] = true;
   fakeProto = "amazon_aws";
   familyInfo[fakeProto] = FamilyLongProtocolPair("Web", "Amazon Web Services");
   enabledInfo[fakeProto] = false;

   qConf.ReconcileWithMaps(familyInfo, enabledInfo);
   conf.updateQosmos(qConf);

   std::stringstream stream;
   conf.writeQosmosToStream(stream);
   MockConf newconf;
   newconf.ReadQosmosFromStringStream(stream);
   QosmosConf pConf = conf.GetQosmosConfigInfo();
   for (int i = 0; i < pConf.qosmosprotocol_size(); i++) {
      const protoMsg::QosmosConf::Protocol& existingProtocol = pConf.qosmosprotocol(i);

      std::string protocolName = existingProtocol.protocolname();
      ASSERT_EQ(familyInfo[protocolName].first, existingProtocol.protocolfamily());
      ASSERT_EQ(familyInfo[protocolName].second, existingProtocol.protocollongname());
      ASSERT_EQ(enabledInfo[protocolName], existingProtocol.protocolenabled());

   }


}

TEST_F(ConfProcessorTests, testConfSlaveWithConfTypeShutdownSuccess) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_type(protoMsg::ConfType_Type_SHUTDOWN);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::ShutdownMsg shutdown;
   shutdown.set_now(true);
   message.push_back(shutdown.SerializeAsString());

   EXPECT_FALSE(slave.ProcessMessage(message));
   EXPECT_TRUE(slave.mShutDownConfReceived);
#endif
}

TEST_F(ConfProcessorTests, testConfSlaveWithConfTypeShutdownFailure) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_type(protoMsg::ConfType_Type_SHUTDOWN);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::ShutdownMsg shutdown;
   message.push_back(shutdown.SerializeAsString());

   EXPECT_FALSE(slave.ProcessMessage(message));
   EXPECT_FALSE(slave.mShutDownConfReceived);
#endif
}

TEST_F(ConfProcessorTests, testConfSlaveWithConfTypeShutdownSuccessOk) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_type(protoMsg::ConfType_Type_SHUTDOWN);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::ShutdownMsg shutdown;
   shutdown.set_now(true); // shutdown ON
   message.push_back(shutdown.SerializeAsString());

   // Test successful value with shutdown:false
   EXPECT_FALSE(slave.ProcessMessage(message));
   EXPECT_TRUE(slave.mShutDownConfReceived);
   EXPECT_TRUE(slave.mShutDownConfValue);
#endif
}

TEST_F(ConfProcessorTests, testConfSlaveWithConfTypeShutdownSuccessFalse) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_type(protoMsg::ConfType_Type_SHUTDOWN);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::ShutdownMsg shutdown;
   shutdown.set_now(false); // shutdown OFF 
   message.push_back(shutdown.SerializeAsString());

   // Test successful value with shutdown:false
   EXPECT_FALSE(slave.ProcessMessage(message));
   EXPECT_TRUE(slave.mShutDownConfReceived);
   EXPECT_FALSE(slave.mShutDownConfValue);
#endif
}

TEST_F(ConfProcessorTests, testConfSlaveRestart) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_type(protoMsg::ConfType_Type_RESTART);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::RestartMsg restartMessage;
   restartMessage.set_restartall(true);
   message.push_back(restartMessage.SerializeAsString());


   EXPECT_FALSE(slave.ProcessMessage(message));
   EXPECT_TRUE(slave.mAppClosed);
#endif
}

TEST_F(ConfProcessorTests, testConfSlaveNoRestart) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_type(protoMsg::ConfType_Type_RESTART);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::RestartMsg restartMessage;
   restartMessage.set_restartall(false);
   message.push_back(restartMessage.SerializeAsString());


   EXPECT_FALSE(slave.ProcessMessage(message));
   EXPECT_FALSE(slave.mAppClosed);
#endif
}

TEST_F(ConfProcessorTests, testConfSlaveBaseConf) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_type(protoMsg::ConfType_Type_BASE);
   configTypeMessage.set_direction(protoMsg::ConfType_Direction_SENDING);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::BaseConf baseMessage;
   baseMessage.set_sendstatsqueue("true");
   message.push_back(baseMessage.SerializeAsString());


   EXPECT_TRUE(slave.ProcessMessage(message));
   EXPECT_FALSE(slave.mAppClosed);
#endif
}
