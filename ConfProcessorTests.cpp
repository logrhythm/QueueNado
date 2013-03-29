/*
 * File:   ConfProcessorTests.cpp
 * Author: Ben Aldrich
 *
 * Created on September 28, 2012, 3:53 PM
 */

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
#include "SyslogConfMsg.pb.h"
#include "Shotgun.h"
#include "g2logworker.hpp"
#include "g2log.hpp"

using namespace std;
using namespace networkMonitor;

TEST_F(ConfProcessorTests, RestartMessagePassedBetweenMasterAndSlave) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   MockConfSlave testSlave;
   testSlave.Start();
   sleep(1);
   
   testSlave.Stop();
   confThread.Stop();
}
TEST_F(ConfProcessorTests, ProcessConfMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_sending(true);

   protoMsg::BaseConf baseConfig;
   std::vector<std::string> shots;
   Conf conf;
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());

   ASSERT_TRUE(conf.EnableIPDefragmentation());
   ASSERT_FALSE(testSlave.ProcessConfMsg(configTypeMessage, shots, conf));
   ASSERT_TRUE(conf.EnableIPDefragmentation());

   shots.clear();
   configTypeMessage.set_conf(false);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_TRUE(conf.EnableIPDefragmentation());
   ASSERT_FALSE(testSlave.ProcessConfMsg(configTypeMessage, shots, conf));
   ASSERT_TRUE(conf.EnableIPDefragmentation());

   shots.clear();
   configTypeMessage.set_conf(true);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_TRUE(conf.EnableIPDefragmentation());
   ASSERT_FALSE(testSlave.ProcessConfMsg(configTypeMessage, shots, conf));
   ASSERT_TRUE(conf.EnableIPDefragmentation());

   shots.clear();
   baseConfig.set_multithreadqosmos("false");
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_TRUE(conf.EnableIPDefragmentation());
   ASSERT_TRUE(testSlave.ProcessConfMsg(configTypeMessage, shots, conf));
   ASSERT_TRUE(conf.EnableIPDefragmentation());

   shots.clear();
   baseConfig.set_qosmosipdefragmentationenabled("false");
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_TRUE(conf.EnableIPDefragmentation());
   ASSERT_TRUE(testSlave.ProcessConfMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(conf.EnableIPDefragmentation());
}



TEST_F(ConfProcessorTests, ProcessQosmosMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_sending(true);

   protoMsg::QosmosConf baseConfig;
   std::vector<std::string> shots;
   MockConf conf;
   conf.InsertFakeQosmosProtocol("test");
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());

   ASSERT_FALSE(testSlave.ProcessQosmosMsg(configTypeMessage, shots, conf));

   shots.clear();
   configTypeMessage.set_qosmosconf(false);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.ProcessQosmosMsg(configTypeMessage, shots, conf));

   shots.clear();
   configTypeMessage.set_qosmosconf(true);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.ProcessQosmosMsg(configTypeMessage, shots, conf));

   shots.clear();
   protoMsg::QosmosConf::Protocol* testProto = baseConfig.add_qosmosprotocol();
   testProto->set_protocolenabled(true);
   testProto->set_protocolname("test");
   testProto->set_protocolfamily("Test");
   testProto->set_protocollongname("This is for Testing");
   ASSERT_EQ("test", baseConfig.qosmosprotocol(0).protocolname());
   ASSERT_TRUE(baseConfig.qosmosprotocol(0).protocolenabled());
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_TRUE(testSlave.ProcessQosmosMsg(configTypeMessage, shots, conf));
   protoMsg::QosmosConf gotConf = conf.getQosmosConfigInfo();
   ASSERT_EQ(1, gotConf.qosmosprotocol_size());
   ASSERT_EQ("test", gotConf.qosmosprotocol(0).protocolname());
   ASSERT_TRUE(gotConf.qosmosprotocol(0).protocolenabled());

}

TEST_F(ConfProcessorTests, ProcessSyslogMsg) {
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_sending(true);

   protoMsg::SyslogConf baseConfig;
   std::vector<std::string> shots;
   Conf conf;
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());

   ASSERT_FALSE(conf.SiemLogging());
   ASSERT_FALSE(testSlave.ProcessSyslogMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(conf.SiemLogging());

   shots.clear();
   configTypeMessage.set_syslogconf(false);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(conf.SiemLogging());
   ASSERT_FALSE(testSlave.ProcessSyslogMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(conf.SiemLogging());

   shots.clear();
   configTypeMessage.set_syslogconf(true);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(conf.SiemLogging());
   ASSERT_FALSE(testSlave.ProcessSyslogMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(conf.SiemLogging());

   shots.clear();
   baseConfig.set_siemlogging("true");
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(conf.SiemLogging());
   ASSERT_TRUE(testSlave.ProcessSyslogMsg(configTypeMessage, shots, conf));
   ASSERT_TRUE(conf.SiemLogging());

   shots.clear();
   baseConfig.set_siemlogging("false");
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_TRUE(conf.SiemLogging());
   ASSERT_TRUE(testSlave.ProcessSyslogMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(conf.SiemLogging());
}

TEST_F(ConfProcessorTests, ProcessRestartMsg) {
#if defined(LR_DEBUG)
   MockConfSlave testSlave;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_sending(true);

   protoMsg::RestartMsg baseConfig;
   std::vector<std::string> shots;
   Conf conf;
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());

   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_FALSE(testSlave.ProcessRestartMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   configTypeMessage.set_restart(false);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_FALSE(testSlave.ProcessRestartMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   configTypeMessage.set_restart(true);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_FALSE(testSlave.ProcessRestartMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   baseConfig.set_restartall(false);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_TRUE(testSlave.ProcessRestartMsg(configTypeMessage, shots, conf));
   ASSERT_FALSE(testSlave.mAppClosed);

   shots.clear();
   baseConfig.set_restartall(true);
   shots.push_back(configTypeMessage.SerializeAsString());
   shots.push_back(baseConfig.SerializeAsString());
   ASSERT_FALSE(testSlave.mAppClosed);
   ASSERT_TRUE(testSlave.ProcessRestartMsg(configTypeMessage, shots, conf));
   ASSERT_TRUE(testSlave.mAppClosed);
#endif
}

TEST_F(ConfProcessorTests, testSingletonInstantiation) {
   ConfMaster& confThread = ConfMaster::Instance();
}

TEST_F(ConfProcessorTests, testNewConfConstructionStream) {
   std::stringstream stream;
   std::stringstream qStream;
   std::stringstream sStream;
   Conf * conf = new Conf(stream, qStream, sStream);
   delete conf;
}

TEST_F(ConfProcessorTests, testNewConfConstructionUpdateMsg) {
   protoMsg::BaseConf msg;
   QosmosConf qmsg;
   protoMsg::SyslogConf sysMsg;
   Conf * conf = new Conf(msg, qmsg, sysMsg);
   delete conf;
}

TEST_F(ConfProcessorTests, getScrubPasswordEnabled) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   //get empty conf
   Conf conf = confThread.GetConf();
   EXPECT_TRUE(conf.getScrubPasswordsEnabled());

}

TEST_F(ConfProcessorTests, testConfIntDefaults) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.Start();
   //get empty conf
   confThread.SetPath(mWriteLocation);
   Conf conf = confThread.GetConf();
   int packetRecv = conf.GetPacketRecvQueueSize();
   int packetSend = conf.GetPacketSendQueueSize();
   int pcapBSize = conf.getPCAPBuffsize();
   int dpiSend = conf.GetDPIMsgSendQueueSize();
   int dpiRecv = conf.GetDPIMsgRecvQueueSize();
   //Expect the defaults that are in the #define
   EXPECT_EQ(PACKET_SEND_QUEUE_SIZE, packetSend);
   EXPECT_EQ(PACKET_RECV_QUEUE_SIZE, packetRecv);
   EXPECT_EQ(PCAP_BUFFER_SIZE, pcapBSize);
   EXPECT_EQ(DPI_MSG_SEND_QUEUE_SIZE, dpiSend);
   EXPECT_EQ(DPI_MSG_RECV_QUEUE_SIZE, dpiRecv);
   EXPECT_EQ(MAX_SYSLOG_LINE_RFC_5426, conf.getSyslogMaxLineLength());
   EXPECT_FALSE(conf.getQosmosDebugModeEnabled());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS, conf.getDpiHalfSessions());
   EXPECT_TRUE(conf.EnableIPDefragmentation());
   EXPECT_TRUE(conf.EnableTCPReassembly());
   EXPECT_TRUE(conf.SingleAppMultiThreadQosmos());
   EXPECT_FALSE(conf.SiemLogging());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS * 5, conf.getQosmos64BytePool());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS * 2.5, conf.getQosmos128BytePool());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS, conf.getQosmos256BytePool());
   EXPECT_EQ(NUMBER_OF_DPI_HALF_SESSIONS * .75, conf.getQosmos512BytePool());
   EXPECT_EQ(DEFAULT_SESSION_EXPIRE_PER_PROCESS, conf.getQosmosExpirePerCallback());
   EXPECT_FALSE(conf.SiemDebugLogging());
   confThread.Stop();
}

TEST_F(ConfProcessorTests, testGetConfFromFile) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   //runs from test/ directory.
   //   Conf conf = confThread.GetConf();
   Conf conf = confThread.GetConf(mTestConf);
   EXPECT_EQ("10.1.1.67", conf.getSyslogAgentIP());
   EXPECT_EQ("514", conf.getSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.getDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/statsAccumulatorQ.ipc", conf.getStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/sendStatsQ.ipc", conf.getSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.getConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.getCommandQueue());
   EXPECT_EQ("/usr/local/nm/logs", conf.getLogDir());
   EXPECT_TRUE(99 == conf.getDpiThreads());
   EXPECT_EQ(123, conf.getPCAPETimeOut());
   EXPECT_EQ(13, conf.getPCAPBuffsize());
   EXPECT_EQ("eth0", conf.getPCAPInterface());
   EXPECT_TRUE(conf.getSyslogEnabled());
   EXPECT_TRUE(conf.getReportEveythingEnabled());
   EXPECT_EQ(2047, conf.getSyslogMaxLineLength());
   EXPECT_EQ(250000, conf.getDpiHalfSessions());
   EXPECT_FALSE(conf.EnableIPDefragmentation());
   EXPECT_FALSE(conf.EnableTCPReassembly());
   EXPECT_EQ(50, conf.GetPacketRecvQueueSize());
   EXPECT_EQ(100, conf.GetPacketSendQueueSize());
   EXPECT_EQ(15000, conf.GetDPIMsgRecvQueueSize());
   EXPECT_EQ(30000, conf.GetDPIMsgSendQueueSize());
   EXPECT_EQ(1, conf.getStatsIntervalSeconds());
   EXPECT_TRUE(conf.getQosmosDebugModeEnabled());
   EXPECT_FALSE(conf.SingleAppMultiThreadQosmos());
   EXPECT_TRUE(conf.SiemLogging());
   EXPECT_TRUE(conf.SiemDebugLogging());
   EXPECT_EQ(64, conf.getQosmos64BytePool());
   EXPECT_EQ(128, conf.getQosmos128BytePool());
   EXPECT_EQ(256, conf.getQosmos256BytePool());
   EXPECT_EQ(512, conf.getQosmos512BytePool());
   EXPECT_EQ(123, conf.getQosmosExpirePerCallback());
   confThread.Stop();

}

TEST_F(ConfProcessorTests, testGetConfFromString) {
   ConfMaster& confThread = ConfMaster::Instance();
   //runs from test/ directory.
   Conf conf = confThread.GetConf(mTestConf);
   EXPECT_EQ("10.1.1.67", conf.getSyslogAgentIP());
   EXPECT_EQ("514", conf.getSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.getDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/statsAccumulatorQ.ipc", conf.getStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/sendStatsQ.ipc", conf.getSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.getConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.getCommandQueue());
   EXPECT_EQ("/usr/local/nm/logs", conf.getLogDir());
   EXPECT_TRUE(99 == conf.getDpiThreads());
   EXPECT_EQ(123, conf.getPCAPETimeOut());
   EXPECT_EQ(13, conf.getPCAPBuffsize());
   EXPECT_EQ("eth0", conf.getPCAPInterface());
   EXPECT_TRUE(conf.getSyslogEnabled());
   EXPECT_TRUE(conf.getReportEveythingEnabled());
   EXPECT_EQ(2047, conf.getSyslogMaxLineLength());
   EXPECT_EQ(1, conf.getStatsIntervalSeconds());
}

TEST_F(ConfProcessorTests, testGetConfInvalidFile) {
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mWriteLocation);
   confThread.Start();
   //runs from test/ directory.
   Conf conf = confThread.GetConf();
   EXPECT_EQ("", conf.getSyslogAgentIP());
   EXPECT_EQ("", conf.getSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.getDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/broadcast.ipc", conf.getBroadcastQueue());
   EXPECT_EQ("ipc:///tmp/statsaccumulator.ipc", conf.getStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/statsmsg.ipc", conf.getSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.getConfChangeQueue());
   EXPECT_EQ("tcp://127.0.0.1:5556", conf.getCommandQueue());
   std::string expectedDir = INSTALL_PREFIX;
   expectedDir += "/logs";
   EXPECT_EQ(expectedDir, conf.getLogDir());
   EXPECT_TRUE(NUMBER_OF_QOSMOS_THREADS == conf.getDpiThreads());
   EXPECT_EQ(PCAP_ETIMEDOUT, conf.getPCAPETimeOut());
   EXPECT_EQ(PCAP_BUFFER_SIZE, conf.getPCAPBuffsize());
   EXPECT_EQ("", conf.getPCAPInterface());
   EXPECT_TRUE(conf.getSyslogEnabled());
   EXPECT_FALSE(conf.getReportEveythingEnabled());
   EXPECT_EQ(MAX_SYSLOG_LINE_RFC_5426, conf.getSyslogMaxLineLength());
   EXPECT_EQ(DEFAULT_STATS_INTERVAL_SEC, conf.getStatsIntervalSeconds());
   EXPECT_EQ("scripts", conf.getScriptsDir());
   confThread.Stop();

}

TEST_F(ConfProcessorTests, testConfSyslogDisabled) {
   protoMsg::SyslogConf msg;
   std::string expSyslogEnabled("false");
   msg.set_syslogenabled(expSyslogEnabled);
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.getSyslogEnabled());
}

TEST_F(ConfProcessorTests, testConfReportEverything) {
   protoMsg::SyslogConf msg;
   std::string reportEverything("true");
   msg.set_reporteverything(reportEverything);
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_TRUE(conf.getReportEveythingEnabled());
}

TEST_F(ConfProcessorTests, testConfQosmosDebugModeEnabled) {
   protoMsg::BaseConf msg;
   std::string qosmosDebugModeEnabled("true");
   msg.set_qosmosdebugmodeenabled(qosmosDebugModeEnabled);
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_TRUE(conf.getQosmosDebugModeEnabled());
}

TEST_F(ConfProcessorTests, testConfQosmos512BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos512bytepool("1234");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(1234, conf.getQosmos512BytePool());
}

TEST_F(ConfProcessorTests, testConfQosmos256BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos256bytepool("12345");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(12345, conf.getQosmos256BytePool());
}

TEST_F(ConfProcessorTests, testConfQosmos128BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos128bytepool("12346");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(12346, conf.getQosmos128BytePool());
}

TEST_F(ConfProcessorTests, testConfQosmos64BytePool) {
   protoMsg::BaseConf msg;
   msg.set_qosmos64bytepool("12347");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(12347, conf.getQosmos64BytePool());
}

TEST_F(ConfProcessorTests, testQosmosExpirePerCallback) {
   protoMsg::BaseConf msg;
   msg.set_qosmosexpirepercallback("111");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ(111, conf.getQosmosExpirePerCallback());
}

TEST_F(ConfProcessorTests, testQosmosTCPReAssembly) {
   protoMsg::BaseConf msg;
   msg.set_qosmostcpreassemblyenabled("false");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.EnableTCPReassembly());
}

TEST_F(ConfProcessorTests, testEnableIPDefragmentation) {
   protoMsg::BaseConf msg;
   msg.set_qosmosipdefragmentationenabled("false");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.EnableIPDefragmentation());
}

TEST_F(ConfProcessorTests, teststatsAccumulatorQueue) {
   protoMsg::BaseConf msg;
   msg.set_statsaccumulatorqueue("12347");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ("12347", conf.getStatsAccumulatorQueue());
}

TEST_F(ConfProcessorTests, teststatsQueue) {
   protoMsg::BaseConf msg;
   msg.set_sendstatsqueue("12347");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ("12347", conf.getSendStatsQueue());
}

TEST_F(ConfProcessorTests, testSingleAppMultiThreadQosmos) {
   protoMsg::BaseConf msg;
   msg.set_multithreadqosmos("true");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   EXPECT_FALSE(conf.SingleAppMultiThreadQosmos());
   conf.updateFields(msg);
   EXPECT_TRUE(conf.SingleAppMultiThreadQosmos());

}

TEST_F(ConfProcessorTests, testSiemLogging) {
   protoMsg::SyslogConf msg;
   msg.set_siemlogging("false");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.SiemLogging());
   msg.set_siemlogging("true");
   conf.updateFields(msg);
   EXPECT_TRUE(conf.SiemLogging());

}

TEST_F(ConfProcessorTests, testCommandQueue) {
   protoMsg::BaseConf msg;
   msg.set_commandqueue("false");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_EQ("false", conf.getCommandQueue());
   msg.set_commandqueue("true");
   conf.updateFields(msg);
   EXPECT_EQ("true", conf.getCommandQueue());

}

TEST_F(ConfProcessorTests, testSiemDebugLogging) {
   protoMsg::SyslogConf msg;
   msg.set_debugsiemlogging("false");
   Conf conf(mTestConf);
   conf.setPath(mWriteLocation);
   conf.updateFields(msg);
   EXPECT_FALSE(conf.SiemDebugLogging());
   msg.set_debugsiemlogging("true");
   conf.updateFields(msg);
   EXPECT_TRUE(conf.SiemDebugLogging());

}

TEST_F(ConfProcessorTests, testProtoMessage) {
   protoMsg::BaseConf msg;
   protoMsg::SyslogConf sysMsg;
   std::string expAgentIP = "24.24.24.24";
   std::string expAgentPort = "514";
   std::string expLogFacility = "local4";
   std::string dpiThreads = "13";
   std::string pcapBufferSize = "30";
   std::string pcapInterface = "foo";
   std::string syslogMaxLineLength = "1234";
   sysMsg.set_sysloglogagentip(expAgentIP);
   sysMsg.set_sysloglogagentport(expAgentPort);
   sysMsg.set_debugsiemlogging("true");
   msg.set_dpithreads(dpiThreads);
   msg.set_pcapetimeout(dpiThreads);
   msg.set_pcapbuffersize(pcapBufferSize);
   msg.set_pcapinterface(pcapInterface);
   sysMsg.set_syslogmaxlinelength(syslogMaxLineLength);
   QosmosConf qmsg;
   Conf conf(msg, qmsg, sysMsg);

   EXPECT_EQ(expAgentIP, conf.getSyslogAgentIP());
   EXPECT_EQ(expAgentPort, conf.getSyslogAgentPort());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(dpiThreads), conf.getDpiThreads());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(dpiThreads), conf.getPCAPETimeOut());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(pcapBufferSize), conf.getPCAPBuffsize());
   EXPECT_EQ(pcapInterface, conf.getPCAPInterface());
   EXPECT_EQ(boost::lexical_cast<unsigned int>(syslogMaxLineLength), conf.getSyslogMaxLineLength());
   EXPECT_TRUE(conf.SiemDebugLogging());
}

TEST_F(ConfProcessorTests, testIpOnlyProtoMessage) {
   protoMsg::SyslogConf sysMsg;
   protoMsg::BaseConf msg;
   std::string expAgentIP = "24.24.24.24";
   sysMsg.set_sysloglogagentip(expAgentIP);
   QosmosConf qmsg;
   Conf conf(msg, qmsg, sysMsg);
   EXPECT_EQ(expAgentIP, conf.getSyslogAgentIP());
   EXPECT_EQ("", conf.getSyslogAgentPort());
}

TEST_F(ConfProcessorTests, testWriteToFile) {
   protoMsg::SyslogConf sysMsg;
   protoMsg::BaseConf msg;
   std::string expAgentIP = "24.24.24.24";
   sysMsg.set_sysloglogagentip(expAgentIP);
   QosmosConf qmsg;
   Conf conf(msg, qmsg, sysMsg);
   std::stringstream stream;
   conf.writeToFile(stream);
   std::stringstream qStream;
   conf.writeQosmosToFile(qStream);
   std::stringstream sStream;
   conf.writeSyslogToFile(sStream);

   Conf newConf(stream, qStream, sStream);

   //TODO:: add compare operator so that we can do EXPECT_EQ(conf, newConf);
   EXPECT_EQ(conf.getSyslogAgentIP(), newConf.getSyslogAgentIP());
}

TEST_F(ConfProcessorTests, testRealChangeAndWriteToDisk) {
   //runs from test/ directory.
   Conf conf(mTestConf);
   EXPECT_EQ("10.1.1.67", conf.getSyslogAgentIP());
   EXPECT_EQ("514", conf.getSyslogAgentPort());

   conf.setPath(mWriteLocation);
   ASSERT_EQ(mWriteLocation, conf.getPath());
   protoMsg::BaseConf msg;
   protoMsg::SyslogConf sysMsg;
   std::string expAgentIP = "24.24.24.24";

   sysMsg.set_sysloglogagentip(expAgentIP);
   conf.updateFields(sysMsg);

   EXPECT_EQ(expAgentIP, conf.getSyslogAgentIP());
   EXPECT_EQ("514", conf.getSyslogAgentPort());

   conf.writeSyslogToFile();

   Conf newConf(mWriteLocation);

   EXPECT_EQ(expAgentIP, newConf.getSyslogAgentIP());
   EXPECT_EQ("514", newConf.getSyslogAgentPort());
   
}

TEST_F(ConfProcessorTests, testPathWithDynamicConf) {
   Conf * conf = new Conf("/path");
   std::string expPath = "/tmp/new_path";
   conf->setPath(expPath);
   EXPECT_EQ(expPath, conf->getPath());
   delete conf;
}

TEST_F(ConfProcessorTests, testGetBaseConfMsg) {

   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf currentConf = confThread.GetConf();
   Crowbar confChangeQ(currentConf.getConfChangeQueue());
   ASSERT_TRUE(confChangeQ.Wield());

   protoMsg::ConfType ctm;
   ctm.set_receiving(true); // Receiving conf from ConfMaster
   ctm.set_conf(true); // Request Base Conf Message
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
   ASSERT_TRUE(ctmRsp.has_sending());
   ASSERT_TRUE(ctmRsp.sending());
   ASSERT_TRUE(ctmRsp.has_conf());
   ASSERT_TRUE(ctmRsp.conf());
   ASSERT_FALSE(ctmRsp.has_receiving());
   ASSERT_FALSE(ctmRsp.has_qosmosconf());

   Conf conf;
   protoMsg::BaseConf confUpdateMsg;
   confUpdateMsg.ParseFromString(data[1]);
   conf.updateFields(confUpdateMsg);
   EXPECT_EQ(5, conf.getStatsIntervalSeconds());

   confThread.Stop();
}

TEST_F(ConfProcessorTests, testGetSyslogConfMsg) {

   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf currentConf = confThread.GetConf();
   Crowbar confChangeQ(currentConf.getConfChangeQueue());
   ASSERT_TRUE(confChangeQ.Wield());

   protoMsg::ConfType ctm;
   ctm.set_receiving(true); // Receiving conf from ConfMaster
   ctm.set_syslogconf(true); // Request Base Conf Message
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
   ASSERT_TRUE(ctmRsp.has_sending());
   ASSERT_TRUE(ctmRsp.sending());
   ASSERT_TRUE(ctmRsp.has_syslogconf());
   ASSERT_TRUE(ctmRsp.syslogconf());
   ASSERT_FALSE(ctmRsp.has_receiving());
   ASSERT_FALSE(ctmRsp.has_conf());
   ASSERT_FALSE(ctmRsp.has_qosmosconf());

   Conf conf;
   protoMsg::SyslogConf confUpdateMsg;
   confUpdateMsg.ParseFromString(data[1]);
   conf.updateFields(confUpdateMsg);
   EXPECT_EQ("10.1.1.67", conf.getSyslogAgentIP());
   EXPECT_EQ("514", conf.getSyslogAgentPort());

   confThread.Stop();
}

TEST_F(ConfProcessorTests, testSendGarbageZmqMsgs) {

   LOG(DEBUG) << "testSendGarbageZmqMsgs begin";
   ConfMaster& confThread = ConfMaster::Instance();
   confThread.SetPath(mTestConf);
   confThread.Start();
   Conf currentConf = confThread.GetConf();
   Crowbar confChangeQ(currentConf.getConfChangeQueue());
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

   EXPECT_EQ("10.1.1.67", conf.getSyslogAgentIP());
   EXPECT_EQ("514", conf.getSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.getDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/statsAccumulatorQ.ipc", conf.getStatsAccumulatorQueue());
   EXPECT_EQ("ipc:///tmp/sendStatsQ.ipc", conf.getSendStatsQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.getConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.getCommandQueue());
   EXPECT_TRUE(99 == conf.getDpiThreads());
   EXPECT_EQ(1, conf.getStatsIntervalSeconds());
   EXPECT_TRUE(conf.getSyslogEnabled());
   EXPECT_EQ("../scripts", conf.getScriptsDir());

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

   Crowbar confChangeQ(conf.getConfChangeQueue());
   //std::cout << conf.getConfChangeQueue() << " sender" << std::endl;
   ASSERT_TRUE(confChangeQ.Wield());

   protoMsg::ConfType ctm;
   ctm.set_sending(true); // Sending conf to ConfMaster
   ctm.set_syslogconf(true); // Base Conf Message
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
   EXPECT_EQ("10.1.1.67", conf.getSyslogAgentIP());
   EXPECT_EQ("514", conf.getSyslogAgentPort());
   EXPECT_EQ("ipc:///tmp/dpilrmsg.ipc", conf.getDpiRcvrQueue());
   EXPECT_EQ("ipc:///tmp/confChangeQ.ipc", conf.getConfChangeQueue());
   EXPECT_EQ("ipc:///tmp/commandQueue.ipc", conf.getCommandQueue());
   EXPECT_EQ("/usr/local/nm/logs", conf.getLogDir());
   EXPECT_TRUE(99 == conf.getDpiThreads());
   EXPECT_EQ(123, conf.getPCAPETimeOut());
   EXPECT_EQ(13, conf.getPCAPBuffsize());
   EXPECT_EQ("eth0", conf.getPCAPInterface());
   EXPECT_TRUE(conf.getSyslogEnabled());
   EXPECT_EQ(2047, conf.getSyslogMaxLineLength());
   slave.Stop();
}

TEST_F(ConfProcessorTests, testConfSlaveUpdate) {
   LOG(DEBUG) << "testConfSlaveUpdate start";
   ConfMaster& master = ConfMaster::Instance();
   ConfSlave& slave = ConfSlave::Instance();
   master.Stop();
   slave.Stop();
   master.SetPath(mWriteLocation);
   slave.SetPath(mWriteLocation);
   master.Start();
   slave.Start();
   sleep(1);
   Conf masterConf = master.GetConf();
   Conf slaveConf = slave.GetConf();
   Conf normalConf = slave.GetConf(mTestConf);

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
   int tries = 5;
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
   EXPECT_EQ(normalConf.getSyslogAgentIP(), slaveConf.getSyslogAgentIP());
   EXPECT_EQ(normalConf.getSyslogAgentPort(), slaveConf.getSyslogAgentPort());
   EXPECT_EQ(normalConf.getDpiRcvrQueue(), slaveConf.getDpiRcvrQueue());
   EXPECT_EQ(normalConf.getConfChangeQueue(), slaveConf.getConfChangeQueue());
   EXPECT_EQ(normalConf.getCommandQueue(), slaveConf.getCommandQueue());
   EXPECT_EQ(normalConf.getDpiThreads(), slaveConf.getDpiThreads());
   EXPECT_EQ(normalConf.getPCAPETimeOut(), slaveConf.getPCAPETimeOut());
   EXPECT_EQ(normalConf.getPCAPBuffsize(), slaveConf.getPCAPBuffsize());
   EXPECT_EQ(normalConf.getPCAPInterface(), slaveConf.getPCAPInterface());
   EXPECT_EQ(normalConf.getSyslogEnabled(), slaveConf.getSyslogEnabled());

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
   Conf conf;
   QosmosConf qConf = conf.getQosmosConfigInfo();
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
   QosmosConf pConf = conf.getQosmosConfigInfo();
   for (int i = 0; i < pConf.qosmosprotocol_size(); i++) {
      const protoMsg::QosmosConf::Protocol& existingProtocol = pConf.qosmosprotocol(i);

      std::string protocolName = existingProtocol.protocolname();
      ASSERT_EQ(familyInfo[protocolName].first, existingProtocol.protocolfamily());
      ASSERT_EQ(familyInfo[protocolName].second, existingProtocol.protocollongname());
      ASSERT_EQ(enabledInfo[protocolName], existingProtocol.protocolenabled());

   }
}

TEST_F(ConfProcessorTests, testWriteQosmosToFile) {
   Conf conf;
   QosmosConf qConf = conf.getQosmosConfigInfo();
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
   conf.writeQosmosToFile(stream);
   Conf newconf;
   newconf.ReadQosmosFromStringStream(stream);
   QosmosConf pConf = conf.getQosmosConfigInfo();
   for (int i = 0; i < pConf.qosmosprotocol_size(); i++) {
      const protoMsg::QosmosConf::Protocol& existingProtocol = pConf.qosmosprotocol(i);

      std::string protocolName = existingProtocol.protocolname();
      ASSERT_EQ(familyInfo[protocolName].first, existingProtocol.protocolfamily());
      ASSERT_EQ(familyInfo[protocolName].second, existingProtocol.protocollongname());
      ASSERT_EQ(enabledInfo[protocolName], existingProtocol.protocolenabled());

   }


}

TEST_F(ConfProcessorTests, testConfSlaveRestart) {
#ifdef LR_DEBUG
   MockConfSlave slave;

   std::vector<std::string> message;
   protoMsg::ConfType configTypeMessage;
   configTypeMessage.set_restart(true);

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
   configTypeMessage.set_restart(true);

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
   configTypeMessage.set_conf(true);
   configTypeMessage.set_sending(true);

   message.push_back(configTypeMessage.SerializeAsString());
   protoMsg::BaseConf baseMessage;
   baseMessage.set_sendstatsqueue("true");
   message.push_back(baseMessage.SerializeAsString());


   EXPECT_TRUE(slave.ProcessMessage(message));
   EXPECT_FALSE(slave.mAppClosed);
#endif
}
