/* 
 * File:   RuleEngineTest.cpp
 * Author: Robert Weber
 * 
 */
#include <iostream>
#include "SendDpiMsgLRZMQ.h"
#include "RuleEngineTest.h"
#include "MockRuleEngine.h"
#include "MockConfSlave.h"
#include "DpiMsgLR.h"
#include "luajit-2.0/lua.hpp"
#include "Vampire.h"
#include "ProcessManager.h"

using namespace networkMonitor;

TEST_F(RuleEngineTest, UpdatePreviousRecordNoLongerLatest) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);

   networkMonitor::DpiMsgLR aMessage;
   aMessage.set_session("abc123");
   aMessage.set_childflownumber(1);
   aMessage.set_timeupdated(123456789);
   dm.UpdatePreviousRecordNoLongerLatest(&aMessage);
   EXPECT_FALSE(dm.mSentUpdate);
   aMessage.set_childflownumber(2);
   dm.UpdatePreviousRecordNoLongerLatest(&aMessage);
   EXPECT_TRUE(dm.mSentUpdate);
   EXPECT_EQ(123456789-600,dm.mEsMessage.timeupdated());
   EXPECT_TRUE(dm.mEsMessage.has_childflownumber());
   EXPECT_EQ(1,dm.mEsMessage.childflownumber());
   EXPECT_FALSE(dm.mEsMessage.latestupdate());
   dm.mSentUpdate = false;

   aMessage.set_timeprevious(123);
   aMessage.set_childflownumber(201);
   dm.UpdatePreviousRecordNoLongerLatest(&aMessage);
   EXPECT_TRUE(dm.mSentUpdate);
   EXPECT_EQ(123,dm.mEsMessage.timeupdated());
   EXPECT_EQ(200,dm.mEsMessage.childflownumber());
   EXPECT_FALSE(dm.mEsMessage.latestupdate());
#endif
}

TEST_F(RuleEngineTest, GetSiemRequiredFieldPairs) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   dm.GetSiemRequiredFieldPairs(tDpiMessage, results);
   ASSERT_EQ(19, results.size());
   EXPECT_EQ("UUID", results[SIEM_FIELD_UUID].first);
   EXPECT_EQ("sip", results[SIEM_FIELD_SIP].first);
   EXPECT_EQ("dip", results[SIEM_FIELD_DIP].first);
   EXPECT_EQ("sport", results[SIEM_FIELD_SPORT].first);
   EXPECT_EQ("dport", results[SIEM_FIELD_DPORT].first);
   EXPECT_EQ("smac", results[SIEM_FIELD_SMAC].first);
   EXPECT_EQ("dmac", results[SIEM_FIELD_DMAC].first);
   EXPECT_EQ("protnum", results[SIEM_FIELD_PROTONUM].first);
   EXPECT_EQ("process", results[SIEM_FIELD_PROCESS].first);
   EXPECT_EQ("bytesin", results[SIEM_FIELD_BYTES_IN].first);
   EXPECT_EQ("deltabytesin", results[SIEM_FIELD_DELTA_BYTES_IN].first);
   EXPECT_EQ("bytesout", results[SIEM_FIELD_BYTES_OUT].first);
   EXPECT_EQ("deltabytesout", results[SIEM_FIELD_DELTA_BYTES_OUT ].first);
   EXPECT_EQ("packetsin", results[SIEM_FIELD_PACKETS_IN].first);
   EXPECT_EQ("deltapacketsin", results[SIEM_FIELD_DELTA_PACKETS_IN].first);
   EXPECT_EQ("TimeStart", results[SIEM_FIELD_TIME_START].first);
   EXPECT_EQ("timeend", results[SIEM_FIELD_TIME_END].first);
   EXPECT_EQ("deltatime", results[SIEM_FIELD_TIME_DELTA].first);
   EXPECT_EQ("totaltime", results[SIEM_FIELD_TIME_TOTAL].first);

   EXPECT_EQ(UNKNOWN_SESSIONID, results[SIEM_FIELD_UUID].second);
   EXPECT_EQ("0.0.0.0", results[SIEM_FIELD_SIP].second);
   EXPECT_EQ("0.0.0.0", results[SIEM_FIELD_DIP].second);
   EXPECT_EQ("0", results[SIEM_FIELD_SPORT].second);
   EXPECT_EQ("0", results[SIEM_FIELD_DPORT].second);
   EXPECT_EQ("00:00:00:00:00:00", results[SIEM_FIELD_SMAC].second);
   EXPECT_EQ("00:00:00:00:00:00", results[SIEM_FIELD_DMAC].second);
   EXPECT_EQ("0", results[SIEM_FIELD_PROTONUM].second);
   EXPECT_EQ("", results[SIEM_FIELD_PROCESS].second); // when we have a "none" field this should change
   EXPECT_EQ("0", results[SIEM_FIELD_BYTES_IN].second);
   EXPECT_EQ("0", results[SIEM_FIELD_DELTA_BYTES_IN].second);
   EXPECT_EQ("0", results[SIEM_FIELD_BYTES_OUT].second);
   EXPECT_EQ("0", results[SIEM_FIELD_DELTA_BYTES_OUT].second);
   EXPECT_EQ("0", results[SIEM_FIELD_PACKETS_IN].second);
   EXPECT_EQ("0", results[SIEM_FIELD_DELTA_PACKETS_IN].second);
   EXPECT_EQ("0", results[SIEM_FIELD_TIME_DELTA].second);
   EXPECT_EQ("0", results[SIEM_FIELD_TIME_TOTAL].second);

   tDpiMessage.set_session("550e8400-e29b-41d4-a716-446655440000");
   tDpiMessage.set_macdest(123);
   tDpiMessage.set_macsource(124);
   tDpiMessage.set_ipdest(125);
   tDpiMessage.set_ipsource(126);
   tDpiMessage.set_portsource(127);
   tDpiMessage.set_portdest(128);
   tDpiMessage.set_protocol(129);
   tDpiMessage.add_application_endq_proto_base("_CHAOSnet");
   tDpiMessage.set_application_id_endq_proto_base(1234);
   tDpiMessage.set_bytesdest(567);
   tDpiMessage.set_bytesdestdelta(456);
   tDpiMessage.set_bytessource(89);
   tDpiMessage.set_bytessourcedelta(78);
   tDpiMessage.set_packettotal(88);
   tDpiMessage.set_packetsdelta(44);
   tDpiMessage.add_loginq_proto_0zz0("dontSeeMee");
   tDpiMessage.set_timestart(1234);
   tDpiMessage.set_timeupdated(5678);
   tDpiMessage.set_timedelta(4444);
   dm.GetSiemRequiredFieldPairs(tDpiMessage, results);
   ASSERT_EQ(19, results.size());
   EXPECT_EQ("UUID", results[SIEM_FIELD_UUID].first);
   EXPECT_EQ("sip", results[SIEM_FIELD_SIP].first);
   EXPECT_EQ("dip", results[SIEM_FIELD_DIP].first);
   EXPECT_EQ("sport", results[SIEM_FIELD_SPORT].first);
   EXPECT_EQ("dport", results[SIEM_FIELD_DPORT].first);
   EXPECT_EQ("smac", results[SIEM_FIELD_SMAC].first);
   EXPECT_EQ("dmac", results[SIEM_FIELD_DMAC].first);
   EXPECT_EQ("protnum", results[SIEM_FIELD_PROTONUM].first);
   EXPECT_EQ("process", results[SIEM_FIELD_PROCESS].first);
   EXPECT_EQ("bytesin", results[SIEM_FIELD_BYTES_IN].first);
   EXPECT_EQ("deltabytesin", results[SIEM_FIELD_DELTA_BYTES_IN].first);
   EXPECT_EQ("bytesout", results[SIEM_FIELD_BYTES_OUT].first);
   EXPECT_EQ("deltabytesout", results[SIEM_FIELD_DELTA_BYTES_OUT ].first);
   EXPECT_EQ("packetsin", results[SIEM_FIELD_PACKETS_IN].first);
   EXPECT_EQ("deltapacketsin", results[SIEM_FIELD_DELTA_PACKETS_IN].first);
   EXPECT_EQ("TimeStart", results[SIEM_FIELD_TIME_START].first);
   EXPECT_EQ("timeend", results[SIEM_FIELD_TIME_END].first);
   EXPECT_EQ("deltatime", results[SIEM_FIELD_TIME_DELTA].first);
   EXPECT_EQ("totaltime", results[SIEM_FIELD_TIME_TOTAL].first);
   EXPECT_EQ("126.0.0.0", results[2].second);
   EXPECT_EQ("125.0.0.0", results[SIEM_FIELD_DIP].second);
   EXPECT_EQ("127", results[SIEM_FIELD_SPORT].second);
   EXPECT_EQ("128", results[SIEM_FIELD_DPORT].second);
   EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", results[SIEM_FIELD_UUID].second);
   EXPECT_EQ("7c:00:00:00:00:00", results[SIEM_FIELD_SMAC].second);
   EXPECT_EQ("7c:00:00:00:00:00", results[SIEM_FIELD_SMAC].second);
   EXPECT_EQ("7b:00:00:00:00:00", results[SIEM_FIELD_DMAC].second);
   EXPECT_EQ("129", results[SIEM_FIELD_PROTONUM].second);
   EXPECT_EQ("26", results[SIEM_FIELD_PROCESS].second);
   EXPECT_EQ("89", results[SIEM_FIELD_BYTES_IN].second);
   EXPECT_EQ("78", results[SIEM_FIELD_DELTA_BYTES_IN].second);
   EXPECT_EQ("567", results[SIEM_FIELD_BYTES_OUT].second);
   EXPECT_EQ("456", results[SIEM_FIELD_DELTA_BYTES_OUT].second);
   EXPECT_EQ("88", results[SIEM_FIELD_PACKETS_IN].second);
   EXPECT_EQ("44", results[SIEM_FIELD_DELTA_PACKETS_IN].second);
   EXPECT_EQ("1234", results[SIEM_FIELD_TIME_START].second);
   EXPECT_EQ("5678", results[SIEM_FIELD_TIME_END].second);
   EXPECT_EQ("4444", results[SIEM_FIELD_TIME_DELTA].second);
   EXPECT_EQ("4444", results[SIEM_FIELD_TIME_TOTAL].second);
#endif
}


TEST_F(RuleEngineTest, ApplicationNameIsTooLarge) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::string tooLarge{"Hello"};
   size_t cutOffSize = tooLarge.size();
   EXPECT_FALSE(dm.ApplicationNameIsTooLarge(cutOffSize+2, tooLarge)); // not too large
   EXPECT_TRUE(dm.ApplicationNameIsTooLarge(cutOffSize+1, tooLarge));
   EXPECT_TRUE(dm.ApplicationNameIsTooLarge(cutOffSize, tooLarge));
   EXPECT_TRUE(dm.ApplicationNameIsTooLarge(cutOffSize-1, tooLarge));
   EXPECT_TRUE(dm.ApplicationNameIsTooLarge(cutOffSize-2, tooLarge));
#endif
}


// Test to make sure that RuleEngine::GetNonNormalizedSiemSyslogMessages does not 
// hang for crazy cut off maximum size message
TEST_F(RuleEngineTest, getSiemSyslogMessagesSplitDataTest__NoHang) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;

   dm.mSiemMode = true;
   dm.mSiemDebugMode = true;
   tDpiMessage.set_session("550e8400-e29b-41d4-a716-446655440000");
   tDpiMessage.set_macdest(123);
   tDpiMessage.set_macsource(124);
   tDpiMessage.set_ipdest(125);
   tDpiMessage.set_ipsource(126);
   tDpiMessage.set_portsource(127);
   tDpiMessage.set_portdest(128);
   tDpiMessage.set_protocol(129);
   tDpiMessage.add_application_endq_proto_base("_CHAOSnet");
   tDpiMessage.set_application_id_endq_proto_base(1234);
   tDpiMessage.set_bytesdest(567);
   tDpiMessage.set_bytesdestdelta(567);
   tDpiMessage.set_bytessource(89);
   tDpiMessage.set_bytessourcedelta(89);
   tDpiMessage.set_packettotal(88);
   tDpiMessage.set_packetsdelta(88);
   tDpiMessage.add_loginq_proto_aim("aLogin");
   tDpiMessage.add_domainq_proto_smb("aDomain1234");
   tDpiMessage.add_uri_fullq_proto_http("this/url.htm");
   tDpiMessage.add_uriq_proto_http("not/this/one");
   tDpiMessage.add_serverq_proto_http("thisname1234");
   tDpiMessage.add_referer_serverq_proto_http("notThisOne");
   // Notice the delimter character '|' placed before LAST.  This forces the
   // application to split the string before the delimiter.
   tDpiMessage.add_methodq_proto_ftp("RUN");
   tDpiMessage.add_methodq_proto_ftp("DOCMD");
   tDpiMessage.add_methodq_proto_ftp("LONGLONGLONG");
   tDpiMessage.add_methodq_proto_ftp("LAST");
   tDpiMessage.add_senderq_proto_smtp("test1_12345");
   tDpiMessage.add_receiverq_proto_smtp("test2_12");
   tDpiMessage.add_subjectq_proto_smtp("test3_1234");
   tDpiMessage.add_versionq_proto_http("4.0");
   tDpiMessage.set_timestart(123);
   tDpiMessage.set_timeupdated(456);
   tDpiMessage.set_timedelta(333);
   dm.SetMaxSize(512 + 8 + 36 + 4);
   messages = dm.GetSiemSyslogMessage(tDpiMessage);
   //   for (int i = 0; i < messages.size(); i++) {
   //      std::cout << messages[i] << std::endl;
   //   }
   ASSERT_EQ(2, messages.size());
   std::string expectedEvent1 = "EVT:001 550e8400-e29b-41d4-a716-446655440000:";
   std::string expectedEvent2 = "EVT:002 550e8400-e29b-41d4-a716-446655440000:";
   std::string expectedStaticData = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,26,89/89,567/567,88/88,123,456,333/333";
   std::string expectedStaticZeroDeltas = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,26,0/89,0/567,0/88,123,456,0/333";
   std::string expected;

   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticData, 0);
   expected += ",login=aLogin,domain=aDomain1234,dname=thisname1234,command=RUN|DOCMD|LONGLONGLONG|LAST,sender=test1_12345,recipient=test2_12,subject=test3_1234,version=4.0,url=this/url.htm,process=_CHAOSnet";
   ASSERT_EQ(expected, messages[0]);

   expected.clear();
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, 1);
   expected += ",application_end=_CHAOSnet,application_id_end=1234,login=aLogin,method=RUN|DOCMD|LONGLONGLONG|LAST,server=thisname1234,referer_server=notThisOne,uri=not/this/one,uri_full=this/url.htm,version=4.0,domain=aDomain1234,sender=test1_12345,receiver=test2_12,subject=test3_1234";
   ASSERT_TRUE(messages.size() > 1);
   ASSERT_EQ(expected, messages[1]);

    
   messages.clear();
   dm.SetMaxSize(167);  
   messages = dm.GetSiemSyslogMessage(tDpiMessage);
   ASSERT_EQ(36, messages.size());
   unsigned int index = 0;
   
   expected.clear();
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticData, index);
   expected += ",login=aLogin";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",domain=aDomain1234";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",dname=thisname1234";
   EXPECT_EQ(expected, messages[index++]);

   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",command=RUN|DOCMD";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",command=LONGLONGLO";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",command=NG|LAST";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",sender=test1_12345";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",recipient=test2_12";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",subject=test3_1234";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",version=4.0";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",url=this/url.htm";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",process=_CHAOSnet";
   EXPECT_EQ(expected, messages[index++]);
   
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=_C";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=HA";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=OS";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=ne";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=t";
   EXPECT_EQ(expected, messages[index++]);
 
   
   // application_id_end is larger than allowed cut-off. 
   // So for this test setting all application_id_end will be ignored
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   EXPECT_EQ(std::string::npos, (messages[index]).find("application_id_end"));
   
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",login=aLogin";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",method=RUN|DOCMD";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",method=LONGLONGLON";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",method=G|LAST";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",server=thisname123";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);

   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",server=4";
   EXPECT_EQ(expected, messages[index++]);
   
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",referer_server=not";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",referer_server=Thi";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",referer_server=sOn";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",referer_server=e";
   EXPECT_EQ(expected, messages[index++]);
   
   
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",uri=not/this/one";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",uri_full=this/url.";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",uri_full=htm";
   EXPECT_EQ(expected, messages[index++]);

   ASSERT_TRUE(index < messages.size());
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",version=4.0";
   ASSERT_TRUE(index < messages.size());
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",domain=aDomain1234";
   ASSERT_TRUE(index < messages.size());
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);// 36
   expected += ",sender=test1_12345";
   ASSERT_TRUE(index < messages.size());
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index); 
   expected += ",receiver=test2_12";
   ASSERT_TRUE(index < messages.size());
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",subject=test3_1234";
   ASSERT_TRUE(index < messages.size());
   EXPECT_EQ(expected, messages[index++]);

#endif
}


TEST_F(RuleEngineTest, getSiemSyslogMessagesSplitDataTest) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;

   dm.mSiemMode = true;
   dm.mSiemDebugMode = true;
   tDpiMessage.set_session("550e8400-e29b-41d4-a716-446655440000");
   tDpiMessage.set_macdest(123);
   tDpiMessage.set_macsource(124);
   tDpiMessage.set_ipdest(125);
   tDpiMessage.set_ipsource(126);
   tDpiMessage.set_portsource(127);
   tDpiMessage.set_portdest(128);
   tDpiMessage.set_protocol(129);
   tDpiMessage.add_application_endq_proto_base("_CHAOSnet");
   tDpiMessage.set_application_id_endq_proto_base(1234);
   tDpiMessage.set_bytesdest(567);
   tDpiMessage.set_bytesdestdelta(567);
   tDpiMessage.set_bytessource(89);
   tDpiMessage.set_bytessourcedelta(89);
   tDpiMessage.set_packettotal(88);
   tDpiMessage.set_packetsdelta(88);
   tDpiMessage.add_loginq_proto_aim("aLogin");
   tDpiMessage.add_domainq_proto_smb("aDomain1234");
   tDpiMessage.add_uri_fullq_proto_http("this/url.htm");
   tDpiMessage.add_uriq_proto_http("not/this/one");
   tDpiMessage.add_serverq_proto_http("thisname1234");
   tDpiMessage.add_referer_serverq_proto_http("notThisOne");
   // Notice the delimter character '|' placed before LAST.  This forces the
   // application to split the string before the delimiter.
   tDpiMessage.add_methodq_proto_ftp("RUN");
   tDpiMessage.add_methodq_proto_ftp("DOCMD");
   tDpiMessage.add_methodq_proto_ftp("LONGLONGLONG");
   tDpiMessage.add_methodq_proto_ftp("LAST");
   tDpiMessage.add_senderq_proto_smtp("test1_12345");
   tDpiMessage.add_receiverq_proto_smtp("test2_12");
   tDpiMessage.add_subjectq_proto_smtp("test3_1234");
   tDpiMessage.add_versionq_proto_http("4.0");
   tDpiMessage.set_timestart(123);
   tDpiMessage.set_timeupdated(456);
   tDpiMessage.set_timedelta(333);
   dm.SetMaxSize(512 + 8 + 36 + 4);
   messages = dm.GetSiemSyslogMessage(tDpiMessage);
   //   for (int i = 0; i < messages.size(); i++) {
   //      std::cout << messages[i] << std::endl;
   //   }
   ASSERT_EQ(2, messages.size());
   std::string expectedEvent1 = "EVT:001 550e8400-e29b-41d4-a716-446655440000:";
   std::string expectedEvent2 = "EVT:002 550e8400-e29b-41d4-a716-446655440000:";
   std::string expectedStaticData = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,26,89/89,567/567,88/88,123,456,333/333";
   std::string expectedStaticZeroDeltas = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,26,0/89,0/567,0/88,123,456,0/333";
   std::string expected;

   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticData, 0);
   expected += ",login=aLogin,domain=aDomain1234,dname=thisname1234,command=RUN|DOCMD|LONGLONGLONG|LAST,sender=test1_12345,recipient=test2_12,subject=test3_1234,version=4.0,url=this/url.htm,process=_CHAOSnet";
   ASSERT_EQ(expected, messages[0]);

   expected.clear();
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, 1);
   expected += ",application_end=_CHAOSnet,application_id_end=1234,login=aLogin,method=RUN|DOCMD|LONGLONGLONG|LAST,server=thisname1234,referer_server=notThisOne,uri=not/this/one,uri_full=this/url.htm,version=4.0,domain=aDomain1234,sender=test1_12345,receiver=test2_12,subject=test3_1234";
   ASSERT_TRUE(messages.size() > 1);
   ASSERT_EQ(expected, messages[1]);

    
   messages.clear();
   dm.SetMaxSize(169);  // Number of chars in SIEM static data, plus first field ",login=aLogin"
   messages = dm.GetSiemSyslogMessage(tDpiMessage);
   ASSERT_EQ(35, messages.size());
   unsigned int index = 0;

   expected.clear();
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticData, index);
   expected += ",login=aLogin";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",domain=aDomain1234";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",dname=thisname1234";
   EXPECT_EQ(expected, messages[index++]);

   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",command=RUN|DOCMD";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",command=LONGLONGLONG";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",command=LAST";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",sender=test1_12345";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",recipient=test2_12";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",subject=test3_1234";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",version=4.0";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",url=this/url.htm";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent1, expectedStaticZeroDeltas, index);
   expected += ",process=_CHAOSnet";
   EXPECT_EQ(expected, messages[index++]);

   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=_CHA";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=OSne";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_end=t";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_id_end=1";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_id_end=2";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_id_end=3";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",application_id_end=4";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",login=aLogin";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",method=RUN|DOCMD";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",method=LONGLONGLONG";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",method=LAST";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",server=thisname1234";
   EXPECT_EQ(expected, messages[index++]);

   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",referer_server=notTh";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",referer_server=isOne";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",uri=not/this/one";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",uri_full=this/url.ht";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",uri_full=m";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",version=4.0";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",domain=aDomain1234";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",sender=test1_12345";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",receiver=test2_12";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent2, expectedStaticZeroDeltas, index);
   expected += ",subject=test3_1234";
   EXPECT_EQ(expected, messages[index++]);   

   EXPECT_EQ(messages.size(), index);

#endif
}

TEST_F(RuleEngineTest, getSiemSyslogMessagesSplitDataTestWithDebug) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;

   dm.mSiemMode = true;
   dm.mSiemDebugMode = false;
   tDpiMessage.set_session("550e8400-e29b-41d4-a716-446655440000");
   tDpiMessage.set_macdest(123);
   tDpiMessage.set_macsource(124);
   tDpiMessage.set_ipdest(125);
   tDpiMessage.set_ipsource(126);
   tDpiMessage.set_portsource(127);
   tDpiMessage.set_portdest(128);
   tDpiMessage.set_protocol(129);
   tDpiMessage.add_application_endq_proto_base("_CHAOSnet");
   tDpiMessage.set_application_id_endq_proto_base(1234);
   tDpiMessage.set_bytesdest(567);
   tDpiMessage.set_bytesdestdelta(567);
   tDpiMessage.set_bytessource(899);
   tDpiMessage.set_bytessourcedelta(899);
   tDpiMessage.set_packettotal(88);
   tDpiMessage.set_packetsdelta(88);
   tDpiMessage.add_loginq_proto_aim("aLogin");
   tDpiMessage.add_domainq_proto_smb("aDomain12345");
   tDpiMessage.add_uri_fullq_proto_http("this/url.htm");
   tDpiMessage.add_uriq_proto_http("not/this/one");
   tDpiMessage.add_serverq_proto_http("thisname12345");
   tDpiMessage.add_referer_serverq_proto_http("notThisOne");
   tDpiMessage.add_methodq_proto_ftp("RUN");
   tDpiMessage.add_methodq_proto_ftp("COMMAND");
   tDpiMessage.add_methodq_proto_ftp("LONGLONGLONGLONG");
   tDpiMessage.add_senderq_proto_smtp("test1_123456");
   tDpiMessage.add_receiverq_proto_smtp("test2_123");
   tDpiMessage.add_subjectq_proto_smtp("test3_12345");
   tDpiMessage.add_versionq_proto_http("4.0");
   tDpiMessage.set_timestart(123);
   tDpiMessage.set_timeupdated(456);
   tDpiMessage.set_timedelta(333);
   int expectedMsgSize(353); // exact size of message with data as defined above
   dm.SetMaxSize(expectedMsgSize);
   messages = dm.GetSiemSyslogMessage(tDpiMessage);
   //  for (int i = 0; i < messages.size(); i++) {
   //      std::cout << messages[i] << ", size: " << messages[i].size() << std::endl;
   //   }
   ASSERT_EQ(1, messages.size());
   ASSERT_EQ(expectedMsgSize, messages[0].size());
   std::string expectedEvent = "EVT:001 550e8400-e29b-41d4-a716-446655440000:";
   std::string expectedHeader = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,26,899/899,567/567,88/88,123,456,333/333";
   std::string expectedHeaderNoCounts = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,26,0/899,0/567,0/88,123,456,0/333";
   std::string expected;
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeader, 0);
   expected += ",login=aLogin,domain=aDomain12345,dname=thisname12345,command=RUN|COMMAND|LONGLONGLONGLONG,sender=test1_123456,recipient=test2_123,subject=test3_12345,version=4.0,url=this/url.htm,process=_CHAOSnet";
   EXPECT_EQ(expected, messages[0]);

   // Force each extra field to be split between multiple syslog EVT:001 messages.
   messages.clear();
   dm.SetMaxSize(169); // Number of chars in SIEM static data, plus first field ",login=aLogin"
   messages = dm.GetSiemSyslogMessage(tDpiMessage);
   //   for (int i = 0; i < messages.size(); i++) {
   //      std::cout << messages[i] << ", size: " << messages[i].size() << std::endl;
   //   }
   ASSERT_EQ(12, messages.size());
   unsigned int index = 0;
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeader, index);
   expected += ",login=aLogin";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",domain=aDomain12345";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",dname=thisname12345";
   EXPECT_EQ(expected, messages[index++]);

   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",command=RUN|COMMAND";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",command=LONGLONGLON";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",command=GLONG";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",sender=test1_123456";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",recipient=test2_123";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",subject=test3_12345";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",version=4.0";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",url=this/url.htm";
   EXPECT_EQ(expected, messages[index++]);
   expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
   expected += ",process=_CHAOSnet";
   EXPECT_EQ(expected, messages[index++]);
#endif
}
#ifdef LR_DEBUG

TEST_F(RuleEngineTest, elasticSearchSends) {
   MockRuleEngine dm(conf, 0);
   std::stringstream targetQueue;

   targetQueue << "ipc:///tmp/ruleEngineTest" << pthread_self();
   dm.SetElasticSearchTarget(targetQueue.str());
   ASSERT_FALSE(dm.CanSendToElasticSearch());
   dm.Start();
   ASSERT_TRUE(dm.CanSendToElasticSearch());
}
#endif

TEST_F(RuleEngineTest, testConstructor) {
#ifdef LR_DEBUG
   conf.SetPath("resources/test.yaml");
   MockRuleEngine re(conf, 0);
#endif
}

TEST_F(RuleEngineTest, testNewDelete) {
#ifdef LR_DEBUG
   conf.SetPath("resources/test.yaml");
   MockRuleEngine * pDpiSyslog = new MockRuleEngine(conf, 0);
   delete pDpiSyslog;
#endif
}

TEST_F(RuleEngineTest, testStartStop) {
#ifdef LR_DEBUG
   conf.SetPath("resources/test.yaml");
   MockRuleEngine re(conf, 0);

   EXPECT_FALSE(re.isRunning());
   re.Start();
   EXPECT_TRUE(re.isRunning());
   re.join();
#endif
}

TEST_F(RuleEngineTest, testMsgReceive) {
#if defined(LR_DEBUG)
   if (geteuid() != 0) {
      //   conf.SetPath("resources/test.yaml");
      //
      protoMsg::BaseConf confMsg;
      protoMsg::SyslogConf sysMsg;

      boost::this_thread::sleep(boost::posix_time::seconds(1));
      MockRuleEngine re(conf, 0);
      re.mSiemMode = false;
      re.Start();
      EXPECT_TRUE(re.isRunning());

      // Send a message to the thread and check the syslog output contains the
      // data send.
      std::string queueName = re.GetDpiRcvrQueue();
      queueName += "0";
      SendDpiMsgLRZMQ sendQueue(queueName);
      sendQueue.Initialize();

      DpiMsgLR msg;

      msg.set_flowtype(DpiMsgLRproto_Type_FINAL);

      std::string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
      msg.set_session(testUuid.c_str());

      std::string testEthSrc("00:22:19:08:2c:00");
      std::vector<unsigned char> ethSrc;
      ethSrc.push_back(0x00);
      ethSrc.push_back(0x22);
      ethSrc.push_back(0x19);
      ethSrc.push_back(0x08);
      ethSrc.push_back(0x2c);
      ethSrc.push_back(0x00);
      msg.SetEthSrc(ethSrc);

      std::string testEthDst("f0:f7:55:dc:a8:00");

      std::vector<unsigned char> ethDst;
      ethDst.push_back(0xf0);
      ethDst.push_back(0xf7);
      ethDst.push_back(0x55);
      ethDst.push_back(0xdc);
      ethDst.push_back(0xa8);
      ethDst.push_back(0x00);
      msg.SetEthDst(ethDst);

      std::string testIpSrc = "10.1.10.50";
      uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
      msg.set_ipsource(ipSrc);

      std::string testIpDst = "10.128.64.251";
      uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
      msg.set_ipdest(ipDst);

      std::string path("base.eth.ip.udp.ntp");
      msg.set_packetpath(path.c_str());

      std::string testIpSourcePort = "=12345"; // bogus, but easier to test
      msg.set_portsource(12345);

      std::string testIpDestPort = "=54321"; // bogus, but easier to test
      msg.set_portdest(54321);

      std::string dataToSend;
      msg.GetBuffer(dataToSend);
      sendQueue.SendData(dataToSend);

      // Allow DPI Syslog Thread time to process the message
      timespec timeToSleep;
      timeToSleep.tv_sec = 0;
      timeToSleep.tv_nsec = 200000000;
      nanosleep(&timeToSleep, NULL);

      //std::cout << "SyslogOutput: " << re.GetSyslogSent().size() << std::endl;
      // Did the data show up in the syslog output
      //      for ( int i = 0 ; i < re.GetSyslogSent().size() ; i++) {
      //          std::cout << re.GetSyslogSent()[i] << std::endl;
      //      }
      ASSERT_EQ(1, re.GetSyslogSent().size());
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("EVT:999 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testUuid));

      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testEthSrc));

      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testEthDst));

      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testIpSrc));

      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testIpDst));

      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(path));

      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testIpSourcePort));

      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testIpDestPort));


      re.join();
   }
#endif
}

TEST_F(RuleEngineTest, testMsgReceiveSiemMode) {
#if defined(LR_DEBUG)
   if (geteuid() != 0) {
      MockConfSlave myConfSlave;

      myConfSlave.SetPath("resources/test.yaml");
      myConfSlave.mConf.mSiemLogging = true;
      //myConfSlave.Start();
      protoMsg::BaseConf confMsg;
      protoMsg::SyslogConf sysMsg;
      sysMsg.set_siemlogging("true");
      MockConfNoMaster myConf = conf.GetConf();
      myConf.UpdateConfigWithMaster(sysMsg);
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      MockRuleEngine re(myConfSlave, 0);
      re.mSiemMode = true;
      re.mSiemDebugMode = false;
      re.Start();
      EXPECT_TRUE(re.isRunning());

      // Send a message to the thread and check the syslog output contains the
      // data send.
      std::string queueName = re.GetDpiRcvrQueue();
      queueName += "0";
      SendDpiMsgLRZMQ sendQueue(queueName);
      sendQueue.Initialize();

      DpiMsgLR msg;

      msg.set_flowtype(DpiMsgLRproto_Type_FINAL);

      std::string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
      msg.set_session(testUuid.c_str());

      std::string testEthSrc("00:22:19:08:2c:00");
      std::vector<unsigned char> ethSrc;
      ethSrc.push_back(0x00);
      ethSrc.push_back(0x22);
      ethSrc.push_back(0x19);
      ethSrc.push_back(0x08);
      ethSrc.push_back(0x2c);
      ethSrc.push_back(0x00);
      msg.SetEthSrc(ethSrc);

      std::string testEthDst("f0:f7:55:dc:a8:00");

      std::vector<unsigned char> ethDst;
      ethDst.push_back(0xf0);
      ethDst.push_back(0xf7);
      ethDst.push_back(0x55);
      ethDst.push_back(0xdc);
      ethDst.push_back(0xa8);
      ethDst.push_back(0x00);
      msg.SetEthDst(ethDst);

      std::string testIpSrc = "10.1.10.50";
      uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
      msg.set_ipsource(ipSrc);

      std::string testIpDst = "10.128.64.251";
      uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
      msg.set_ipdest(ipDst);

      std::string path("base.eth.ip.udp.ntp");
      msg.set_packetpath(path.c_str());

      std::string testIpSourcePort = "=12345"; // bogus, but easier to test
      msg.set_portsource(12345);

      std::string testIpDestPort = "=54321"; // bogus, but easier to test
      msg.set_portdest(54321);
      msg.set_protocol(12);
      msg.set_application_id_endq_proto_base(13);
      msg.add_application_endq_proto_base("wrong");
      msg.add_application_endq_proto_base("_3Com_Corp");
      msg.set_bytesdest(12345);
      msg.set_bytessource(6789);
      msg.set_packettotal(99);
      msg.add_loginq_proto_aim("aLogin");
      msg.add_domainq_proto_smb("aDomain");
      msg.add_uri_fullq_proto_http("this/url.htm");
      msg.add_uriq_proto_http("notitUrl");
      msg.add_serverq_proto_http("thisname");
      msg.add_referer_serverq_proto_http("notitServer");
      msg.add_methodq_proto_ftp("TEST");
      msg.add_methodq_proto_ftp("COMMAND");
      msg.add_senderq_proto_smtp("test1");
      msg.add_receiverq_proto_smtp("test2");
      msg.add_subjectq_proto_smtp("test3");
      msg.add_versionq_proto_http("4.0");
      msg.add_filenameq_proto_gnutella("aFilename");
      msg.add_filename_encodingq_proto_aim_transfer("notitFile");
      msg.add_directoryq_proto_smb("aPath");
      msg.set_timestart(123);
      msg.set_timeupdated(456);
      msg.set_sessionidq_proto_ymsg(2345);
      std::string dataToSend;
      msg.GetBuffer(dataToSend);
      sendQueue.SendData(dataToSend);

      // Allow DPI Syslog Thread time to process the message
      timespec timeToSleep;
      timeToSleep.tv_sec = 0;
      timeToSleep.tv_nsec = 200000000;
      nanosleep(&timeToSleep, NULL);

      //std::cout << "SyslogOutput: " << re.GetSyslogSent().size() << std::endl;
      // Did the data show up in the syslog output
      ASSERT_EQ(1, re.GetSyslogSent().size());
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("EVT:001 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testUuid));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,6789/6789,12345/12345,99/99,123,456,333/333"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("login=aLogin"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("domain=aDomain"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("url=this/url.htm"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("dname=thisname"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("command=TEST|COMMAND"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("sender=test1"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("recipient=test2"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("subject=test3"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("version=4.0"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("objectname=aFilename"));
      EXPECT_EQ(std::string::npos, re.GetSyslogSent()[0].find("notit"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("object=aPath"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("session=2345"));

      re.join();
   }
#endif
}

TEST_F(RuleEngineTest, testMsgReceiveIntermediateTypes) {
#if defined(LR_DEBUG)
   if (geteuid() != 0) {
      MockConfSlave myConfSlave;

      myConfSlave.SetPath("resources/test.yaml");
      myConfSlave.mConf.mSiemLogging = true;
      //myConfSlave.Start();
      protoMsg::BaseConf confMsg;
      protoMsg::SyslogConf sysMsg;
      sysMsg.set_siemlogging("true");
      MockConfNoMaster myConf = conf.GetConf();
      myConf.UpdateConfigWithMaster(sysMsg);
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      MockRuleEngine re(myConfSlave, 0);
      re.mSiemMode = true;
      re.mSiemDebugMode = false;
      re.Start();
      EXPECT_TRUE(re.isRunning());

      // Send a message to the thread and check the syslog output contains the
      // data send.
      std::string queueName = re.GetDpiRcvrQueue();
      queueName += "0";
      SendDpiMsgLRZMQ sendQueue(queueName);
      sendQueue.Initialize();

      DpiMsgLR msg;

      msg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE);

      std::string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
      msg.set_session(testUuid.c_str());

      std::string testEthSrc("00:22:19:08:2c:00");
      std::vector<unsigned char> ethSrc;
      ethSrc.push_back(0x00);
      ethSrc.push_back(0x22);
      ethSrc.push_back(0x19);
      ethSrc.push_back(0x08);
      ethSrc.push_back(0x2c);
      ethSrc.push_back(0x00);
      msg.SetEthSrc(ethSrc);

      std::string testEthDst("f0:f7:55:dc:a8:00");

      std::vector<unsigned char> ethDst;
      ethDst.push_back(0xf0);
      ethDst.push_back(0xf7);
      ethDst.push_back(0x55);
      ethDst.push_back(0xdc);
      ethDst.push_back(0xa8);
      ethDst.push_back(0x00);
      msg.SetEthDst(ethDst);

      std::string testIpSrc = "10.1.10.50";
      uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
      msg.set_ipsource(ipSrc);

      std::string testIpDst = "10.128.64.251";
      uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
      msg.set_ipdest(ipDst);

      std::string path("base.eth.ip.udp.ntp");
      msg.set_packetpath(path.c_str());

      std::string testIpSourcePort = "=12345"; // bogus, but easier to test
      msg.set_portsource(12345);

      std::string testIpDestPort = "=54321"; // bogus, but easier to test
      msg.set_portdest(54321);
      msg.set_protocol(12);
      msg.set_application_id_endq_proto_base(13);
      msg.add_application_endq_proto_base("wrong");
      msg.add_application_endq_proto_base("_3Com_Corp");
      msg.set_bytesdest(12345);
      msg.set_bytessource(6789);
      msg.set_packettotal(99);
      msg.add_loginq_proto_aim("aLogin");
      msg.add_domainq_proto_smb("aDomain");
      msg.add_uri_fullq_proto_http("this/url.htm");
      msg.add_uriq_proto_http("notitUrl");
      msg.add_serverq_proto_http("thisname");
      msg.add_referer_serverq_proto_http("notitServer");
      msg.add_methodq_proto_ftp("TEST");
      msg.add_methodq_proto_ftp("COMMAND");
      msg.add_senderq_proto_smtp("test1");
      msg.add_receiverq_proto_smtp("test2");
      msg.add_subjectq_proto_smtp("test3");
      msg.add_versionq_proto_http("4.0");
      msg.add_filenameq_proto_gnutella("aFilename");
      msg.add_filename_encodingq_proto_aim_transfer("notitFile");
      msg.add_directoryq_proto_smb("aPath");
      msg.set_timestart(123);
      msg.set_timeupdated(456); // delta = 333
      msg.set_sessionidq_proto_ymsg(2345);
      std::string dataToSend;
      msg.GetBuffer(dataToSend);
      sendQueue.SendData(dataToSend);

      msg.set_bytesdest(23456); // delta = 11111
      msg.set_bytessource(7900); // delta = 1111
      msg.set_packettotal(210); // delta = 111
      msg.set_timeupdated(567); // delta = 111
      dataToSend.clear();
      msg.GetBuffer(dataToSend);
      sendQueue.SendData(dataToSend);

      msg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE_FINAL);
      msg.set_bytesdest(45678); // delta = 22222
      msg.set_bytessource(10122); // delta = 2222
      msg.set_packettotal(432); // delta = 222
      msg.set_timeupdated(789); // delta = 222
      dataToSend.clear();
      msg.GetBuffer(dataToSend);
      sendQueue.SendData(dataToSend);

      // Allow DPI Syslog Thread time to process the messages
      timespec timeToSleep;
      timeToSleep.tv_sec = 0;
      timeToSleep.tv_nsec = 200000000;
      nanosleep(&timeToSleep, NULL);

      //std::cout << "SyslogOutput: " << re.GetSyslogSent().size() << std::endl;
      // Did the data show up in the syslog output
      ASSERT_EQ(4, re.GetSyslogSent().size());
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("EVT:003 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testUuid));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,6789/6789,12345/12345,99/99,123,456,333/333"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("EVT:003 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find(testUuid));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,1111/7900,11111/23456,111/210,123,567,111/444"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[2].find("EVT:003 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[2].find(testUuid));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[2].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,2222/10122,22222/45678,222/432,123,789,222/666"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[3].find("EVT:001 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[3].find(testUuid));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[3].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,0/10122,0/45678,0/432,123,789,0/666"));
      re.join();
   }
#endif
}

TEST_F(RuleEngineTest, testMsgIntermediateFinalNoIntermediate) {
#if defined(LR_DEBUG)
   if (geteuid() != 0) {
      MockConfSlave myConfSlave;

      myConfSlave.SetPath("resources/test.yaml");
      myConfSlave.mConf.mSiemLogging = false;
      //myConfSlave.Start();
      protoMsg::BaseConf confMsg;
      protoMsg::SyslogConf sysMsg;
      sysMsg.set_siemlogging("true");
      MockConfNoMaster myConf = conf.GetConf();
      myConf.UpdateConfigWithMaster(sysMsg);
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      MockRuleEngine re(myConfSlave, 0);
      re.mSiemMode = true;
      re.mSiemDebugMode = false;
      re.Start();
      EXPECT_TRUE(re.isRunning());

      // Send a message to the thread and check the syslog output contains the
      // data send.
      std::string queueName = re.GetDpiRcvrQueue();
      queueName += "0";
      SendDpiMsgLRZMQ sendQueue(queueName);
      sendQueue.Initialize();

      DpiMsgLR msg;

      std::string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
      msg.set_session(testUuid.c_str());

      std::string testEthSrc("00:22:19:08:2c:00");
      std::vector<unsigned char> ethSrc;
      ethSrc.push_back(0x00);
      ethSrc.push_back(0x22);
      ethSrc.push_back(0x19);
      ethSrc.push_back(0x08);
      ethSrc.push_back(0x2c);
      ethSrc.push_back(0x00);
      msg.SetEthSrc(ethSrc);

      std::string testEthDst("f0:f7:55:dc:a8:00");

      std::vector<unsigned char> ethDst;
      ethDst.push_back(0xf0);
      ethDst.push_back(0xf7);
      ethDst.push_back(0x55);
      ethDst.push_back(0xdc);
      ethDst.push_back(0xa8);
      ethDst.push_back(0x00);
      msg.SetEthDst(ethDst);

      std::string testIpSrc = "10.1.10.50";
      uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
      msg.set_ipsource(ipSrc);

      std::string testIpDst = "10.128.64.251";
      uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
      msg.set_ipdest(ipDst);

      std::string path("base.eth.ip.udp.ntp");
      msg.set_packetpath(path.c_str());

      std::string testIpSourcePort = "=12345"; // bogus, but easier to test
      msg.set_portsource(12345);

      std::string testIpDestPort = "=54321"; // bogus, but easier to test
      msg.set_portdest(54321);
      msg.set_protocol(12);
      msg.set_application_id_endq_proto_base(13);
      msg.add_application_endq_proto_base("wrong");
      msg.add_application_endq_proto_base("_3Com_Corp");
      msg.set_bytesdest(12345);
      msg.set_bytessource(6789);
      msg.set_packettotal(99);
      msg.add_loginq_proto_aim("aLogin");
      msg.add_domainq_proto_smb("aDomain");
      msg.add_uri_fullq_proto_http("this/url.htm");
      msg.add_uriq_proto_http("notitUrl");
      msg.add_serverq_proto_http("thisname");
      msg.add_referer_serverq_proto_http("notitServer");
      msg.add_methodq_proto_ftp("TEST");
      msg.add_methodq_proto_ftp("COMMAND");
      msg.add_senderq_proto_smtp("test1");
      msg.add_receiverq_proto_smtp("test2");
      msg.add_subjectq_proto_smtp("test3");
      msg.add_versionq_proto_http("4.0");
      msg.add_filenameq_proto_gnutella("aFilename");
      msg.add_filename_encodingq_proto_aim_transfer("notitFile");
      msg.add_directoryq_proto_smb("aPath");
      msg.set_timestart(123);
      msg.set_timeupdated(456); // delta = 333
      msg.set_sessionidq_proto_ymsg(2345);

      msg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE_FINAL);
      std::string dataToSend;
      msg.GetBuffer(dataToSend);
      sendQueue.SendData(dataToSend);

      // Allow DPI Syslog Thread time to process the messages
      timespec timeToSleep;
      timeToSleep.tv_sec = 0;
      timeToSleep.tv_nsec = 200000000;
      nanosleep(&timeToSleep, NULL);

      // for (int i = 0; i < re.GetSyslogSent().size(); i++) {
      //    std::cout << re.GetSyslogSent()[i] << ", size: " << re.GetSyslogSent()[i].size() << std::endl;
      // }
      // The data should not show up in the syslog output
      ASSERT_EQ(2, re.GetSyslogSent().size());
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("EVT:003 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testUuid));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,6789/6789,12345/12345,99/99,123,456,333/333"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("EVT:001 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find(testUuid));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,0/6789,0/12345,0/99,123,456,0/333"));

      re.join();
   }
#endif
}

TEST_F(RuleEngineTest, testMsgReceiveSiemModeDebug) {
#if defined(LR_DEBUG)
   if (geteuid() != 0) {
      MockConfSlave myConfSlave;

      myConfSlave.SetPath("resources/test.yaml");
      myConfSlave.mConf.mSiemLogging = true;
      myConfSlave.mConf.mSiemDebug = true;
      //myConfSlave.Start();
      protoMsg::BaseConf confMsg;
      protoMsg::SyslogConf sysMsg;
      sysMsg.set_siemlogging("true");
      MockConfNoMaster myConf = conf.GetConf();
      myConf.UpdateConfigWithMaster(sysMsg);
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      MockRuleEngine re(myConfSlave, 0);
      re.mSiemMode = true;

      re.mSiemDebugMode = true;
      re.Start();
      EXPECT_TRUE(re.isRunning());

      // Send a message to the thread and check the syslog output contains the
      // data send.
      std::string queueName = re.GetDpiRcvrQueue();
      queueName += "0";
      SendDpiMsgLRZMQ sendQueue(queueName);
      sendQueue.Initialize();

      DpiMsgLR msg;

      msg.set_flowtype(DpiMsgLRproto_Type_FINAL);

      std::string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
      msg.set_session(testUuid.c_str());

      std::string testEthSrc("00:22:19:08:2c:00");
      std::vector<unsigned char> ethSrc;
      ethSrc.push_back(0x00);
      ethSrc.push_back(0x22);
      ethSrc.push_back(0x19);
      ethSrc.push_back(0x08);
      ethSrc.push_back(0x2c);
      ethSrc.push_back(0x00);
      msg.SetEthSrc(ethSrc);

      std::string testEthDst("f0:f7:55:dc:a8:00");

      std::vector<unsigned char> ethDst;
      ethDst.push_back(0xf0);
      ethDst.push_back(0xf7);
      ethDst.push_back(0x55);
      ethDst.push_back(0xdc);
      ethDst.push_back(0xa8);
      ethDst.push_back(0x00);
      msg.SetEthDst(ethDst);

      std::string testIpSrc = "10.1.10.50";
      uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
      msg.set_ipsource(ipSrc);

      std::string testIpDst = "10.128.64.251";
      uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
      msg.set_ipdest(ipDst);

      std::string path("base.eth.ip.udp.ntp");
      msg.set_packetpath(path.c_str());

      std::string testIpSourcePort = "=12345"; // bogus, but easier to test
      msg.set_portsource(12345);

      std::string testIpDestPort = "=54321"; // bogus, but easier to test
      msg.set_portdest(54321);
      msg.set_protocol(12);
      msg.set_application_id_endq_proto_base(13);
      msg.add_application_endq_proto_base("wrong");
      msg.add_application_endq_proto_base("_3Com_Corp");
      msg.set_bytesdest(12345);
      msg.set_bytessource(67890);
      msg.set_packettotal(99);
      msg.add_loginq_proto_aim("aLogin");
      msg.add_domainq_proto_smb("aDomain");
      msg.add_uri_fullq_proto_http("this/url.htm");
      msg.add_uriq_proto_http("notitUrl");
      msg.add_serverq_proto_http("thisname");
      msg.add_referer_serverq_proto_http("notitServer");
      msg.add_methodq_proto_ftp("TEST");
      msg.add_methodq_proto_ftp("COMMAND");
      msg.add_senderq_proto_smtp("test1");
      msg.add_receiverq_proto_smtp("test2");
      msg.add_subjectq_proto_smtp("test3");
      msg.add_versionq_proto_http("4.0");
      msg.add_filenameq_proto_gnutella("aFilename");
      msg.add_filename_encodingq_proto_aim_transfer("notitFile");
      msg.add_directoryq_proto_smb("aPath");
      msg.set_timestart(123);
      msg.set_timeupdated(456);
      msg.set_sessionidq_proto_ymsg(2345);
      std::string dataToSend;
      msg.GetBuffer(dataToSend);
      sendQueue.SendData(dataToSend);

      // Allow DPI Syslog Thread time to process the message
      timespec timeToSleep;
      timeToSleep.tv_sec = 0;
      timeToSleep.tv_nsec = 200000000;
      nanosleep(&timeToSleep, NULL);

      //std::cout << "SyslogOutput: " << re.GetSyslogSent().size() << std::endl;
      // Did the data show up in the syslog output
      //      for ( int i = 0 ; i < re.GetSyslogSent().size() ; i++) {
      //          std::cout << re.GetSyslogSent()[i] << std::endl;
      //      }
      ASSERT_EQ(2, re.GetSyslogSent().size());
      std::string testUuidWithNumber = testUuid;
      testUuidWithNumber += ":00";
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("EVT:001 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find(testUuidWithNumber));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,67890/67890,12345/12345,99/99,123,456,333/333"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("login=aLogin"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("domain=aDomain"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("url=this/url.htm"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("dname=thisname"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("command=TEST|COMMAND"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("sender=test1"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("recipient=test2"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("subject=test3"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("version=4.0"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("objectname=aFilename"));
      EXPECT_EQ(std::string::npos, re.GetSyslogSent()[0].find("notit"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("object=aPath"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[0].find("session=2345"));

      testUuidWithNumber = testUuid;
      testUuidWithNumber += ":01";
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("EVT:002 "));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find(testUuidWithNumber));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,2,0/67890,0/12345,0/99,123,456,0/333"));
      EXPECT_EQ(std::string::npos, re.GetSyslogSent()[1].find("EndTime=456"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("application_end=wrong|_3Com_Corp"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("applidation_id_end=13"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("login=aLogin"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("filenameEncoding=notitFile"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("method=TEST|COMMAND"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("filename=aFilename"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("server=thisname"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("referer_server=notitServer"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("uri=notitUrl"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("uri_full=this/url.htm"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("version=4.0"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("directory=aPath"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("domain=aDomain"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("sender=test1"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("receiver=test2"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("subject=test3"));
      EXPECT_NE(std::string::npos, re.GetSyslogSent()[1].find("Session=2345"));

      re.join();
   }
#endif
}

TEST_F(RuleEngineTest, getSyslogMessagesBufferTooSmall) {
#ifdef LR_DEBUG
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;
   MockRuleEngine dm(conf, 0);

   dataPairs[0] = std::make_pair("bad0", "one");
   dataPairs[1] = std::make_pair("test0", "me0");
   dataPairs[2] = std::make_pair("test1", "me1");
   dataPairs[3] = std::make_pair("test2", "me2");
   dataPairs[4] = std::make_pair("test3", "me3");
   dataPairs[5] = std::make_pair("test4", "me4");
   dataPairs[6] = std::make_pair("test5", "me5");
   dataPairs[7] = std::make_pair("bad2", "one2");
   dataPairs[10000] = std::make_pair("bad2", "one3");
   dm.SetMaxSize(11 * 5 + 8);
   ASSERT_FALSE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
#endif
}

TEST_F(RuleEngineTest, getSyslogMessages) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair<std::string, std::string> > results;

   tDpiMessage.set_ipsource(0x0a0b0c0d);
   tDpiMessage.set_ipdest(0x01020304);
   tDpiMessage.set_macsource(0x00000a0b0c0d0e0f);
   tDpiMessage.set_macdest(0x0000010203040506);
   tDpiMessage.set_session("01234567-89ab-cdef-0123456789abcdef");
   std::string path = "foo.bar";
   tDpiMessage.set_packetpath(path);
   tDpiMessage.set_portsource(1234);
   tDpiMessage.set_portdest(5678);

   tDpiMessage.set_filesizeq_proto_bittorrent(1212);
   tDpiMessage.set_filesizeq_proto_edonkey(12345678901234L);
   tDpiMessage.add_attach_filenameq_proto_facebook_mail("this is a file");

   results = tDpiMessage.GetAllFieldsAsStrings();
   ASSERT_EQ(11, results.size());
   std::vector<std::string> syslogMessages;
   std::map<unsigned int, std::pair<std::string, std::string> > formattedFieldData =
           tDpiMessage.GetAllFieldsAsStrings();
   dm.SetMaxSize(2048);
   ASSERT_TRUE(dm.GetSyslogMessages(formattedFieldData, syslogMessages, DYNAMIC_DATA_START));
   ASSERT_EQ(1, syslogMessages.size());
   EXPECT_EQ("EVT:999 UUID=01234567-89ab-cdef-0123456789abcdef, "
           "EthSrc=0f:0e:0d:0c:0b:0a, EthDst=06:05:04:03:02:01, "
           "IpSrc=13.12.11.10, IpDst=4.3.2.1, Path=foo.bar, "
           "SourcePort=1234, DestPort=5678, filesize=1212, "
           "filesize=12345678901234, attach_filename=this is a file",
           syslogMessages[0]);
#endif

}

TEST_F(RuleEngineTest, getSyslogMessagesBigMessage) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;

   dataPairs[1] = std::make_pair("0", "0");
   dataPairs[2] = std::make_pair("1", "1");
   dataPairs[3] = std::make_pair("2", "2");
   dataPairs[4] = std::make_pair("3", "3");
   dataPairs[5] = std::make_pair("4", "4");
   dataPairs[6] = std::make_pair("5", "5");
   dataPairs[8] = std::make_pair("6", "6");
   dataPairs[9] = std::make_pair("7", "7");
   dataPairs[DYNAMIC_DATA_START] = std::make_pair("a", "10");
   dataPairs[DYNAMIC_DATA_START + 1] = std::make_pair("b", "123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa|bbbbbbbb");
   dataPairs[DYNAMIC_DATA_START + 2] = std::make_pair("c", "1");

   dm.SetMaxSize((7 * 5 + 3) + 8 + 8);

   ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
   //   for (int i = 0 ; i < messages.size(); i++) {
   //      std::cout << messages[i] << std::endl;
   //   }
   ASSERT_EQ(3, messages.size());
   ASSERT_EQ("EVT:999 0=0, 1=1, 2=2, 3=3, 4=4, 5=5, 6=6, 7=7, a=10", messages[0]);
   ASSERT_EQ("EVT:999 0=0, b=123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa", messages[1]);
   ASSERT_EQ("EVT:999 0=0, b=bbbbbbbb, c=1", messages[2]);
#endif
}

TEST_F(RuleEngineTest, getSyslogMessagesSplitDataTest) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;

   int currentDynamicField = DYNAMIC_DATA_START;
   dataPairs[1] = std::make_pair("UUID", "57c4384a-15b7-44c0-9814-b2e95b23dd15");
   dataPairs[2] = std::make_pair("EthSrc", "f0:f7:55:dc:a8:7f");
   dataPairs[3] = std::make_pair("EthDst", "84:18:88:7b:db:04");
   dataPairs[4] = std::make_pair("IpSrc", "10.128.24.59");
   dataPairs[5] = std::make_pair("IpDst", "192.168.178.21");
   dataPairs[6] = std::make_pair("Path", "base.eth.ip.tcp|base.eth.ip.tcp.http");
   dataPairs[7] = std::make_pair("SourcePort", "52421");
   dataPairs[8] = std::make_pair("DestPort", "80");
   dataPairs[9] = std::make_pair("FlowCompleted", "true");
   dataPairs[10] = std::make_pair("Application", "tcp|http");
   dataPairs[11] = std::make_pair("flowId", "49649");
   dataPairs[12] = std::make_pair("family", "Network Service|Web");
   dataPairs[13] = std::make_pair("applicationId", "67");
   dataPairs[14] = std::make_pair("session", "300");
   dataPairs[15] = std::make_pair("dev", "eth0");
   dataPairs[16] = std::make_pair("declassified", "67");
   dataPairs[17] = std::make_pair("application_end", "tcp|http");
   dataPairs[18] = std::make_pair("familyEnd", "Network Service|Web");
   dataPairs[19] = std::make_pair("applidation_id_end", "67");
   dataPairs[20] = std::make_pair("sessionLen", "74256");
   dataPairs[21] = std::make_pair("server", "192.168.178.21");
   dataPairs[currentDynamicField++] = std::make_pair("referer", "http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/");
   dataPairs[currentDynamicField++] = std::make_pair("referer", "http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/");
   dataPairs[currentDynamicField++] = std::make_pair("referer_server", "192.168.178.21");
   dataPairs[currentDynamicField++] = std::make_pair("uri", "/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel");
   dataPairs[currentDynamicField++] = std::make_pair("uri", "uri=/ha/status_json|/activity/query/");
   dataPairs[currentDynamicField++] = std::make_pair("uri_full", "/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel");
   dataPairs[currentDynamicField++] = std::make_pair("userAgent", "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E; MDDR)");
   dataPairs[currentDynamicField++] = std::make_pair("mime", "text/html");
   dataPairs[currentDynamicField++] = std::make_pair("method", "GET");
   dataPairs[currentDynamicField++] = std::make_pair("version", "1.1");
   dataPairs[currentDynamicField++] = std::make_pair("serverAgent", "Apache/2.2.14 (Ubuntu) mod_ssl/2.2.14 OpenSSL/0.9.8k mod_fastcgi/2.4.6");
   dataPairs[currentDynamicField++] = std::make_pair("rttQ_PROTO_HTTP", "0.236118|0.720132|0.721577|0.726061|0.730292|0.712930|0.717017|0.689391|0.744431|0.725071|0.732237|0.719680|0.720685|0.731514|0.733816|0.763178|0.219743|0.719475|0.723441|0.734603|0.706938|0.692968|0.658081|0.731333|0.739257|0.737810|0.725861|0.731982|0.727546|0.721841|0.704078|0.709424|0.232954|0.764327|0.717029|0.724730|0.715977|0.705920|0.763877|0.728564|0.738548|0.727176|0.714079|0.714557|0.694057|0.726929|0.701185|0.727373|0.227517|0.699752|0.765659|0.732002|0.736440|0.720890|0.724230|0.719586|0.729707|0.712549|0.715832|0.723022|0.722323|0.722087|0.712564|0.714766|0.725085|0.722592|0.718138|0.732483|0.732279|0.758076|0.711826|0.730520|0.216783|0.718176|0.719815|0.711295|0.694291|0.771365|0.722658|0.707148|0.729696|0.717143|0.718489|0.714692|0.712587|0.736150|0.719579|0.705759|0.216820|0.760850|0.703886|0.710497|0.720210|0.712620|0.733677|0.716371|0.722511|0.712855|0.764938|0.702055|0.678128");
   dataPairs[currentDynamicField++] = std::make_pair("header_nameQ_PROTO_HTTP", "headerName=x-requested-with|Accept-Language|Referer|Accept|Accept-Encoding|User-Agent|Host|Connection|Date|Server|Content-Length|Expires|Pragma|Cache-Control|Set-Cookie|Keep-Alive|Content-Type|Transfer-Encoding");
   dataPairs[currentDynamicField++] = std::make_pair("header_valueQ_PROTO_HTTP", "XMLHttpRequest|en-us|http://192.168.178.21/frameset/upper/http://192.168.178.21/frameset/upper/|application/json, text/javascript, */*|gzip, deflate");
   dataPairs[currentDynamicField++] = std::make_pair("long_field_valueQ_PROTO_BOGUS", "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_|Next field beyond 520 boundary");

   dm.SetMaxSize(2048);
   ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
   ASSERT_EQ(2, messages.size());

   // Try a different max size.
   messages.clear();
   dm.SetMaxSize(1024);
   ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
   ASSERT_EQ(5, messages.size());

   // Try a different max size.
   messages.clear();
   dm.SetMaxSize(512 + 8);

   ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
   //   for (int i = 0; i < messages.size(); i++) {
   //      std::cout << messages[i] << ", size: " << messages[i].size() << std::endl;
   //   }
   ASSERT_EQ(8, messages.size());
   std::string expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, EthSrc=f0:f7:55:dc:a8:7f, EthDst=84:18:88:7b:db:04, IpSrc=10.128.24.59, IpDst=192.168.178.21, Path=base.eth.ip.tcp|base.eth.ip.tcp.http, DestPort=80, FlowCompleted=true, Application=tcp|http, flowId=49649, family=Network Service|Web, applicationId=67, session=300, dev=eth0, declassified=67, application_end=tcp|http, familyEnd=Network Service|Web, applidation_id_end=67, sessionLen=74256, server=192.168.178.21";
   EXPECT_EQ(expected, messages[0]);
   expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, referer=http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/, referer=http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/, referer_server=192.168.178.21, uri=/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel, uri=uri=/ha/status_json|/activity/query/, uri_full=/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel";
   EXPECT_EQ(expected, messages[1]);
   expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, userAgent=Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E; MDDR), mime=text/html, method=GET, version=1.1, serverAgent=Apache/2.2.14 (Ubuntu) mod_ssl/2.2.14 OpenSSL/0.9.8k mod_fastcgi/2.4.6";
   EXPECT_EQ(expected, messages[2]);
   expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, rttQ_PROTO_HTTP=0.236118|0.720132|0.721577|0.726061|0.730292|0.712930|0.717017|0.689391|0.744431|0.725071|0.732237|0.719680|0.720685|0.731514|0.733816|0.763178|0.219743|0.719475|0.723441|0.734603|0.706938|0.692968|0.658081|0.731333|0.739257|0.737810|0.725861|0.731982|0.727546|0.721841|0.704078|0.709424|0.232954|0.764327|0.717029|0.724730|0.715977|0.705920|0.763877|0.728564|0.738548|0.727176|0.714079|0.714557|0.694057|0.726929|0.701185|0.727373|0.227517|0.699752";
   EXPECT_EQ(expected, messages[3]);
   expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, rttQ_PROTO_HTTP=0.765659|0.732002|0.736440|0.720890|0.724230|0.719586|0.729707|0.712549|0.715832|0.723022|0.722323|0.722087|0.712564|0.714766|0.725085|0.722592|0.718138|0.732483|0.732279|0.758076|0.711826|0.730520|0.216783|0.718176|0.719815|0.711295|0.694291|0.771365|0.722658|0.707148|0.729696|0.717143|0.718489|0.714692|0.712587|0.736150|0.719579|0.705759|0.216820|0.760850|0.703886|0.710497|0.720210|0.712620|0.733677|0.716371|0.722511|0.712855|0.764938|0.702055";
   EXPECT_EQ(expected, messages[4]);
   expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, rttQ_PROTO_HTTP=0.678128, header_nameQ_PROTO_HTTP=headerName=x-requested-with|Accept-Language|Referer|Accept|Accept-Encoding|User-Agent|Host|Connection|Date|Server|Content-Length|Expires|Pragma|Cache-Control|Set-Cookie|Keep-Alive|Content-Type|Transfer-Encoding, header_valueQ_PROTO_HTTP=XMLHttpRequest|en-us|http://192.168.178.21/frameset/upper/http://192.168.178.21/frameset/upper/|application/json, text/javascript, */*|gzip, deflate";
   EXPECT_EQ(expected, messages[5]);
   expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, long_field_valueQ_PROTO_BOGUS=123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789";
   EXPECT_EQ(expected, messages[6]);
   expected = "EVT:999 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, long_field_valueQ_PROTO_BOGUS=_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_|Next field beyond 520 boundary";
   EXPECT_EQ(expected, messages[7]);

#endif
}

TEST_F(RuleEngineTest, CrazyData) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;
   int currentDynamicField = DYNAMIC_DATA_START;
   dataPairs[1] = std::make_pair("0", "0");
   dataPairs[2] = std::make_pair("1", "1");
   dataPairs[3] = std::make_pair("2", "2");
   dataPairs[4] = std::make_pair("3", "3");
   dataPairs[5] = std::make_pair("4", "4");
   dataPairs[6] = std::make_pair("5", "5");
   dataPairs[8] = std::make_pair("6", "6");
   dataPairs[9] = std::make_pair("7", "7");
   dataPairs[currentDynamicField++] = std::make_pair("a", "10");
   dataPairs[currentDynamicField++] = std::make_pair("b", "123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa|bbbbbbbb1234567890123456789012345678900aaaa");
   dm.SetMaxSize((7 * 5 + 3) + 8 + 8);
   ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
   //      for (int i = 0 ; i < messages.size(); i++) {
   //   std::cout << messages[i] << std::endl;
   //   }
   ASSERT_EQ(4, messages.size());
   ASSERT_EQ("EVT:999 0=0, 1=1, 2=2, 3=3, 4=4, 5=5, 6=6, 7=7, a=10", messages[0]);
   ASSERT_EQ("EVT:999 0=0, b=123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa", messages[1]);
   ASSERT_EQ("EVT:999 0=0, b=bbbbbbbb1234567890123456789012345678900", messages[2]);
   ASSERT_EQ("EVT:999 0=0, b=aaaa", messages[3]);
#endif
}

TEST_F(RuleEngineTest, getSyslogMessagesLongMessage) {
#if defined(LR_DEBUG)
   MockRuleEngine dm(conf, 0);
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   std::vector<std::string> messages;
   int currentDynamicField = DYNAMIC_DATA_START;
   dataPairs[1] = std::make_pair("0", "0");
   dataPairs[2] = std::make_pair("1", "1");
   dataPairs[3] = std::make_pair("2", "2");
   dataPairs[4] = std::make_pair("3", "3");
   dataPairs[5] = std::make_pair("4", "4");
   dataPairs[6] = std::make_pair("5", "5");
   dataPairs[8] = std::make_pair("6", "6");
   dataPairs[9] = std::make_pair("7", "7");
   dataPairs[currentDynamicField++] = std::make_pair("a", "1");
   dataPairs[currentDynamicField++] = std::make_pair("b", "1");
   dataPairs[currentDynamicField++] = std::make_pair("c", "1");
   dataPairs[currentDynamicField++] = std::make_pair("d", "1");
   dataPairs[currentDynamicField++] = std::make_pair("e", "1");
   dataPairs[currentDynamicField++] = std::make_pair("f", "1");
   dm.SetMaxSize((7 * 5 + 3) + 10 + 8);
   ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));

   ASSERT_EQ(2, messages.size());
   ASSERT_EQ("EVT:999 0=0, 1=1, 2=2, 3=3, 4=4, 5=5, 6=6, 7=7, a=1, b=1", messages[0]);
   ASSERT_EQ("EVT:999 0=0, c=1, d=1, e=1, f=1", messages[1]);
#endif
}

TEST_F(RuleEngineTest, getStaticInfo) {
#ifdef LR_DEBUG
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   MockRuleEngine dm(conf, 0);
   int currentDynamicField = DYNAMIC_DATA_START;
   dataPairs[0] = std::make_pair("bad", "one");
   dataPairs[1] = std::make_pair("test", "me");
   dataPairs[2] = std::make_pair("test1", "me1");
   dataPairs[3] = std::make_pair("test2", "me2");
   dataPairs[4] = std::make_pair("test3", "me3");
   dataPairs[5] = std::make_pair("test4", "me4");
   dataPairs[6] = std::make_pair("test5", "me5");
   dataPairs[8] = std::make_pair("test6", "me6");
   dataPairs[9] = std::make_pair("test7", "me7");
   dataPairs[7] = std::make_pair("bad", "one2");
   dataPairs[10000] = std::make_pair("bad", "one3");

   ASSERT_EQ("test=me, test1=me1, test2=me2, test3=me3, test4=me4, test5=me5, test6=me6, test7=me7", dm.GetStaticInfo(dataPairs, DYNAMIC_DATA_START));
#endif
}

TEST_F(RuleEngineTest, getNextDataPair) {
#ifdef LR_DEBUG
   std::map<unsigned int, std::pair <std::string, std::string> > dataPairs;
   MockRuleEngine dm(conf, 0);

   dataPairs[0] = std::make_pair("test", "me");
   auto i = dataPairs.begin();
   ASSERT_EQ(", test=me", dm.GetNextDataPair(i));
#endif

}

TEST_F(RuleEngineTest, GetLoginField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_loginq_proto_0zz0("test1");
   ASSERT_EQ(2, dm.GetLoginField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("login", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_loginq_proto_0zz0("test1");
   tDpiMessage.add_loginq_proto_0zz0("test2");
   tDpiMessage.add_loginq_proto_0zz0("test3");
   ASSERT_EQ(2, dm.GetLoginField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_loginq_proto_aim("test1");
   ASSERT_EQ(2, dm.GetLoginField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("login", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_contact_loginq_proto_aim("test1");
   ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_proxy_loginq_proto_http("test1");
   ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_login_serverq_proto_imap("test1");
   ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, GetDomainField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetDomainField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_domainq_proto_smb("test1");
   ASSERT_EQ(2, dm.GetDomainField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("domain", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_real_sender_domainq_proto_gmail("test1");
   ASSERT_EQ(2, dm.GetDomainField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("domain", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_real_sender_domainq_proto_gmail("test1");
   tDpiMessage.add_real_sender_domainq_proto_gmail("test2");
   tDpiMessage.add_real_sender_domainq_proto_gmail("test3");
   ASSERT_EQ(2, dm.GetDomainField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetDomainField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_vtp_mgmt_domainq_proto_cdp("test1");
   ASSERT_EQ(2, dm.GetDomainField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("domain", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_caller_domainq_proto_sip("not_sent");
   ASSERT_EQ(1, dm.GetDomainField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();

   tDpiMessage.add_callee_domainq_proto_sip("not_sent");
   ASSERT_EQ(1, dm.GetDomainField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, GetDestHostField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetDestHostField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_serverq_proto_http("test1");
   ASSERT_EQ(2, dm.GetDestHostField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("dname", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_serverq_proto_http("test1");
   tDpiMessage.add_serverq_proto_http("test2");
   tDpiMessage.add_serverq_proto_http("test3");
   ASSERT_EQ(2, dm.GetDestHostField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetDestHostField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_destination_hostq_proto_diameter("test1");
   ASSERT_EQ(2, dm.GetDestHostField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("dname", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_origin_hostq_proto_diameter("test1");
   ASSERT_EQ(1, dm.GetDestHostField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.set_server_addrq_proto_ip("test1");
   ASSERT_EQ(1, dm.GetDestHostField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, GetUrlField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetUrlField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_uriq_proto_http("notit");
   tDpiMessage.add_uri_fullq_proto_http("test1");
   ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("url", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_uri_pathq_proto_http("notit");
   tDpiMessage.add_uriq_proto_http("test1");
   tDpiMessage.add_destination_hostq_proto_diameter("test1");
   ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("url", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_uriq_proto_http("test1");
   tDpiMessage.add_uriq_proto_http("test2");
   tDpiMessage.add_uriq_proto_http("test3");
   ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetUrlField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_ad_urlq_proto_google_ads("notit");
   tDpiMessage.add_ad_url_fullq_proto_google_ads("test1");
   tDpiMessage.add_destination_hostq_proto_diameter("test1");
   ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("url", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_ad_urlq_proto_google_ads("test1");
   tDpiMessage.add_ad_visible_urlq_proto_google_ads("notit");
   ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("url", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_urilast64q_proto_rtsp("notit");
   ASSERT_EQ(1, dm.GetUrlField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_ad_visible_urlq_proto_google_ads("test1");
   ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("url", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, GetCommandField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetCommandField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_command_stringq_proto_smb("test1");
   ASSERT_EQ(2, dm.GetCommandField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("command", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_methodq_proto_ftp("test1");
   ASSERT_EQ(2, dm.GetCommandField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("command", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_methodq_proto_ftp("test1");
   tDpiMessage.add_methodq_proto_ftp("test2");
   tDpiMessage.add_methodq_proto_ftp("test3");
   ASSERT_EQ(2, dm.GetCommandField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetCommandField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();
   results.clear();

   tDpiMessage.set_command_codeq_proto_diameter(1);
   ASSERT_EQ(1, dm.GetCommandField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_h245_methodq_proto_h225("notit");
   ASSERT_EQ(1, dm.GetCommandField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetSenderField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetSenderField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_senderq_proto_aim("test1");
   ASSERT_EQ(2, dm.GetSenderField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("sender", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_senderq_proto_aim("test1");
   tDpiMessage.add_senderq_proto_aim("test2");
   tDpiMessage.add_senderq_proto_aim("test3");
   ASSERT_EQ(2, dm.GetSenderField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetSenderField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();
   results.clear();
   tDpiMessage.add_msglist_sender_aliasq_proto_dimp("notit");
   ASSERT_EQ(1, dm.GetSenderField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetRecipientField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetRecipientField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_receiverq_proto_aim("test1");
   ASSERT_EQ(2, dm.GetRecipientField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("recipient", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_receiverq_proto_aim("test1");
   tDpiMessage.add_receiverq_proto_aim("test2");
   tDpiMessage.add_receiverq_proto_aim("test3");
   ASSERT_EQ(2, dm.GetRecipientField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetRecipientField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
   results.clear();
   tDpiMessage.add_msglist_receiver_aliasq_proto_dimp("notit");
   ASSERT_EQ(1, dm.GetSenderField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetSubjectField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetSubjectField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_subjectq_proto_smtp("test1");
   ASSERT_EQ(2, dm.GetSubjectField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("subject", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_subjectq_proto_smtp("test1");
   tDpiMessage.add_subjectq_proto_smtp("test2");
   tDpiMessage.add_subjectq_proto_smtp("test3");
   ASSERT_EQ(2, dm.GetSubjectField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetSubjectField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();
   results.clear();
   tDpiMessage.add_msglist_subjectq_proto_dimp("notit");
   ASSERT_EQ(1, dm.GetSubjectField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetVersionField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetVersionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.set_versionq_proto_rpc(21);
   ASSERT_EQ(2, dm.GetVersionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("version", results[1].first);
   EXPECT_EQ("21", results[1].second);
   results.clear();
   tDpiMessage.Clear();
   tDpiMessage.add_versionq_proto_skype("test1");
   ASSERT_EQ(2, dm.GetVersionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("version", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();
   tDpiMessage.add_server_versionq_proto_teamspeak("test1");
   ASSERT_EQ(2, dm.GetVersionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("version", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_server_versionq_proto_teamspeak("test1");
   tDpiMessage.add_server_versionq_proto_teamspeak("test2");
   tDpiMessage.add_server_versionq_proto_teamspeak("test3");
   ASSERT_EQ(2, dm.GetVersionField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetVersionField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
   results.clear();
   tDpiMessage.set_program_versionq_proto_rpc(99);
   ASSERT_EQ(1, dm.GetVersionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetPathField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetPathField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_directoryq_proto_http("aPath");
   ASSERT_EQ(2, dm.GetPathField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("object", results[1].first);
   EXPECT_EQ("aPath", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_directoryq_proto_http("aPath");
   tDpiMessage.add_directoryq_proto_http("bPath");
   tDpiMessage.add_directoryq_proto_http("cPath");
   ASSERT_EQ(2, dm.GetPathField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("aPath|bPath|cPath", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetPathField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();
   results.clear();
   tDpiMessage.add_directoryq_proto_smb("test1");
   ASSERT_EQ(2, dm.GetPathField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("object", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();


#endif
}

TEST_F(RuleEngineTest, GetFilenameField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetFilenameField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_filenameq_proto_winmx("aName");
   ASSERT_EQ(2, dm.GetFilenameField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("objectname", results[1].first);
   EXPECT_EQ("aName", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_filenameq_proto_winmx("aName");
   tDpiMessage.add_filenameq_proto_winmx("bName");
   tDpiMessage.add_filenameq_proto_winmx("cName");
   ASSERT_EQ(2, dm.GetFilenameField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("aName|bName|cName", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetFilenameField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();
   results.clear();
   tDpiMessage.add_attach_filenameq_proto_yandex_webmail("test1");
   ASSERT_EQ(2, dm.GetFilenameField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("objectname", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_bootfilenameq_proto_dhcp("notit");
   ASSERT_EQ(1, dm.GetFilenameField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());


#endif
}

TEST_F(RuleEngineTest, GetSessionField) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.set_sessionidq_proto_ymsg(1234);
   ASSERT_EQ(2, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("session", results[1].first);
   EXPECT_EQ("1234", results[1].second);
   results.clear();
   tDpiMessage.Clear();
   tDpiMessage.add_session_idq_proto_ymail2("test1");
   ASSERT_EQ(2, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("session", results[1].first);
   EXPECT_EQ("test1", results[1].second);
   results.clear();
   tDpiMessage.Clear();

   tDpiMessage.add_session_idq_proto_ymail2("test1");
   tDpiMessage.add_session_idq_proto_ymail2("test2");
   tDpiMessage.add_session_idq_proto_ymail2("test3");
   ASSERT_EQ(2, dm.GetSessionField(1, tDpiMessage, 3, results));
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("test1|test2|test3", results[1].second);
   results.clear();
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 4, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.Clear();
   results.clear();
   tDpiMessage.set_session_durationq_proto_sip("test1");
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
   tDpiMessage.add_end_sessionq_proto_rtp("test1");
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
   tDpiMessage.set_bytessource(1234);
   tDpiMessage.set_bytesdest(5678);
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
   tDpiMessage.set_session_packet_counterq_proto_base(1234);
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());
   results.clear();
   tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, EmptyLongFieldsTest) {
#ifdef LR_DEBUG
   MockRuleEngine dm(conf, 0);
   IndexedFieldPairs results;
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.add_session_idq_proto_ymail2("test1");
   tDpiMessage.add_session_idq_proto_ymail2("test2");
   tDpiMessage.add_session_idq_proto_ymail2("test3");
   tDpiMessage.add_session_idq_proto_ymail2("test4");
   tDpiMessage.add_session_idq_proto_ymail2("test5");
   tDpiMessage.add_session_idq_proto_ymail2("test6");
   tDpiMessage.add_session_idq_proto_ymail2("test7");
   tDpiMessage.add_session_idq_proto_ymail2("test8");
   tDpiMessage.add_session_idq_proto_ymail2("test9");
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 10, results));
   ASSERT_EQ(0, results.size());
   tDpiMessage.add_session_idq_proto_ymail2("test10");
   ASSERT_EQ(2, dm.GetSessionField(1, tDpiMessage, 10, results));
   ASSERT_EQ(1, results.size());
   results.clear();
   dm.EmptyLongFields(tDpiMessage);
   ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, 0, results));
   ASSERT_EQ(0, results.size());

   tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, SyslogSendingQueue) {
#ifdef LR_DEBUG
   MockRuleEngine re(conf, 0);

   re.SetSyslogIntercept(false);
   std::string testNoCrash("Call SendToSyslogReporter with uninitialized queue");
   re.SendToSyslogReporter(testNoCrash);
   EXPECT_TRUE(re.InitializeSyslogSendQueue());
   std::string queueName = conf.GetConf().GetSyslogQueue();
   Vampire rcvQueue(queueName);
   rcvQueue.SetHighWater(100);
   rcvQueue.SetIOThreads(1);
   rcvQueue.SetOwnSocket(true);
   EXPECT_TRUE(rcvQueue.PrepareToBeShot());
   std::string testSendMsg("Test Message");
   re.SendToSyslogReporter(testSendMsg);
   std::string testRcvMsg;
   EXPECT_TRUE(rcvQueue.GetShot(testRcvMsg, 500));
   EXPECT_EQ(testSendMsg, testRcvMsg);

#endif
}

