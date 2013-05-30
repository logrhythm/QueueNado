#include <stdlib.h>
#include "LuaFunctionsTest.h"
#include "MockConf.h"
#include "g2logworker.hpp"
#include "g2log.hpp"
#include "MockLuaExecuter.h"
#include "LuaPacketFunctions.h"
#include "LuaRuleEngineFunctions.h"
#include "RuleEngine.h"
#include "MockRuleEngine.h"

static int LuaTestFunction(lua_State *L) {
   return 0;
}

static int LuaTestFunction2(lua_State *L) {
   return 0;
}

TEST_F(LuaFunctionsTest, AddRemoveFromContainer) {
   MockLuaFunctions functions;

   ASSERT_EQ(0, functions.NumberOfFunctions());
   functions.AddFunction("Test1", LuaTestFunction);
   ASSERT_EQ(1, functions.NumberOfFunctions());
   functions.AddFunction("Test1", LuaTestFunction2);
   ASSERT_EQ(1, functions.NumberOfFunctions());
   functions.RemoveFunction("Test1");
   ASSERT_EQ(0, functions.NumberOfFunctions());
}

TEST_F(LuaFunctionsTest, AddThenRegister) {
   MockLuaFunctions functions;

   ASSERT_EQ(0, functions.NumberOfFunctions());
   functions.AddFunction("Test1", LuaTestFunction);
   ASSERT_EQ(1, functions.NumberOfFunctions());
   functions.AddFunction("Test2", LuaTestFunction2);
   ASSERT_EQ(2, functions.NumberOfFunctions());

   MockLuaExecuter ruleEngine;
   functions.RegisterAllKnownFunctions(ruleEngine);

   std::map<std::string, lua_CFunction> registered = ruleEngine.GetPossibleFunctions();

   ASSERT_TRUE(registered["Test1"] == LuaTestFunction);
   ASSERT_TRUE(registered["Test2"] == LuaTestFunction2);
}

TEST_F(LuaFunctionsTest, PacketFunctions) {
   LuaPacketFunctions* functions = new LuaPacketFunctions;
   MockConf conf;
   std::stringstream testDir;
   testDir << "/tmp/TooMuchPcap." << pthread_self();

   conf.mPCapCaptureLocation = testDir.str();

   std::string makeADir = "mkdir -p ";
   makeADir += testDir.str();

   ASSERT_EQ(0, system(makeADir.c_str()));
   functions->StartPacketCapture(conf);
   MockLuaExecuter ruleEngine;
   functions->RegisterAllKnownFunctions(ruleEngine);

   std::map<std::string, lua_CFunction> registered = ruleEngine.GetPossibleFunctions();
   ASSERT_TRUE(registered.end() != registered.find("SessionAge"));
   ASSERT_TRUE(registered.end() != registered.find("EndOfFlow"));
   ASSERT_TRUE(registered.end() != registered.find("GetCurrentClassification"));
   ASSERT_TRUE(registered.end() != registered.find("CapturePacket"));
   ASSERT_TRUE(registered.end() != registered.find("WriteAllCapturedPackets"));
   ASSERT_TRUE(registered.end() != registered.find("CleanupCapturedPackets"));

   ASSERT_EQ(registered["SessionAge"], LuaPacketFunctions::LuaSessionAge);
   ASSERT_EQ(registered["EndOfFlow"], LuaPacketFunctions::LuaDataNull);
   ASSERT_EQ(registered["GetCurrentClassification"], LuaPacketFunctions::LuaDpiMsgClassification);
   ASSERT_EQ(registered["CapturePacket"], LuaPacketFunctions::LuaBufferSessionToMemory);
   ASSERT_EQ(registered["WriteAllCapturedPackets"], LuaPacketFunctions::LuaWriteAllPacketsToDisk);
   ASSERT_EQ(registered["CleanupCapturedPackets"], LuaPacketFunctions::LuaEmptyBufferOfSession);
   delete functions;
}

TEST_F(LuaFunctionsTest, LuaSessionAge) {
   networkMonitor::DpiMsgLR dpiMsg;

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, 10);
   LuaPacketFunctions::LuaSessionAge(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));
   lua_close(luaState);
   luaState = luaL_newstate();
   dpiMsg.set_endtime(std::time(NULL));
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, 999);
   LuaPacketFunctions::LuaSessionAge(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));
   lua_close(luaState);
   luaState = luaL_newstate();
   time_t pasttime = std::time(NULL) - 11;
   dpiMsg.set_endtime(pasttime);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, 10);
   LuaPacketFunctions::LuaSessionAge(luaState);
   EXPECT_TRUE(lua_toboolean(luaState, -1));
   EXPECT_TRUE(dpiMsg.flowtype() == ::networkMonitor::DpiMsgLRproto_Type_INTERMEDIATE);
   EXPECT_NE(pasttime, dpiMsg.endtime());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, LuaDataNull) {
   networkMonitor::DpiMsgLR dpiMsg;

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::LuaDataNull(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));
   lua_close(luaState);
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, NULL);
   LuaPacketFunctions::LuaDataNull(luaState);
   EXPECT_TRUE(lua_toboolean(luaState, -1));

   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, LuaDpiMsgClassification) {
   networkMonitor::DpiMsgLR dpiMsg;

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::LuaDpiMsgClassification(luaState);
   std::string classification(lua_tostring(luaState, -1));
   EXPECT_EQ("unknown", classification);
   lua_close(luaState);
   luaState = luaL_newstate();
   dpiMsg.add_applicationq_proto_base("test1");
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::LuaDpiMsgClassification(luaState);
   classification=lua_tostring(luaState, -1);
   EXPECT_EQ("test1", classification);
   lua_close(luaState);
   dpiMsg.add_application_endq_proto_base("test2");
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::LuaDpiMsgClassification(luaState);
   classification=lua_tostring(luaState, -1);
   EXPECT_EQ("test2", classification);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, RuleEngineFunctions) {
   LuaRuleEngineFunctions* functions = new LuaRuleEngineFunctions;
   MockConf conf;

   MockLuaExecuter ruleEngine;
   functions->RegisterAllKnownFunctions(ruleEngine);

   std::map<std::string, lua_CFunction> registered = ruleEngine.GetPossibleFunctions();

   ASSERT_TRUE(registered.end() != registered.find("IsIntermediateFlow"));
   ASSERT_TRUE(registered.end() != registered.find("IsIntermediateFinalFlow"));
   ASSERT_TRUE(registered.end() != registered.find("IsFinalFlow"));
   ASSERT_TRUE(registered.end() != registered.find("GetUuid"));
   ASSERT_TRUE(registered.end() != registered.find("GetLatestApplication"));
   ASSERT_TRUE(registered.end() != registered.find("GetPacketCount"));
   ASSERT_TRUE(registered.end() != registered.find("SetDeltaPackets"));
   ASSERT_TRUE(registered.end() != registered.find("GetSessionLenServer"));
   ASSERT_TRUE(registered.end() != registered.find("SetDeltaSessionLenServer"));
   ASSERT_TRUE(registered.end() != registered.find("GetSessionLenClient"));
   ASSERT_TRUE(registered.end() != registered.find("SetDeltaSessionLenClient"));
   ASSERT_TRUE(registered.end() != registered.find("GetStartTime"));
   ASSERT_TRUE(registered.end() != registered.find("GetEndTime"));
   ASSERT_TRUE(registered.end() != registered.find("SetDeltaTime"));
   ASSERT_TRUE(registered.end() != registered.find("GetDeltaTime"));
   ASSERT_TRUE(registered.end() != registered.find("SendInterFlow"));
   ASSERT_TRUE(registered.end() != registered.find("SendFinalFlow"));
   ASSERT_TRUE(registered.end() != registered.find("SendAccStat"));
   ASSERT_TRUE(registered.end() != registered.find("GetThreadId"));
   ASSERT_TRUE(registered.end() != registered.find("GetRawMsgSize"));
   ASSERT_TRUE(registered.end() != registered.find("GetDpiMsgSize"));
   ASSERT_TRUE(registered.end() != registered.find("SendStat"));
   ASSERT_EQ(registered["IsIntermediateFlow"], LuaRuleEngineFunctions::LuaIsIntermediateFlow);
   ASSERT_EQ(registered["IsIntermediateFinalFlow"], LuaRuleEngineFunctions::LuaIsIntermediateFinalFlow);
   ASSERT_EQ(registered["IsFinalFlow"], LuaRuleEngineFunctions::LuaIsFinalFlow);
   ASSERT_EQ(registered["GetUuid"], LuaRuleEngineFunctions::LuaGetUuid);
   ASSERT_EQ(registered["GetLatestApplication"], LuaRuleEngineFunctions::LuaGetLatestApplication);
   ASSERT_EQ(registered["GetPacketCount"], LuaRuleEngineFunctions::LuaGetPacketCount);
   ASSERT_EQ(registered["SetDeltaPackets"], LuaRuleEngineFunctions::LuaSetDeltaPackets);
   ASSERT_EQ(registered["GetSessionLenServer"], LuaRuleEngineFunctions::LuaGetSessionLenServer);
   ASSERT_EQ(registered["SetDeltaSessionLenServer"], LuaRuleEngineFunctions::LuaSetDeltaSessionLenServer);
   ASSERT_EQ(registered["GetSessionLenClient"], LuaRuleEngineFunctions::LuaGetSessionLenClient);
   ASSERT_EQ(registered["SetDeltaSessionLenClient"], LuaRuleEngineFunctions::LuaSetDeltaSessionLenClient);
   ASSERT_EQ(registered["GetStartTime"], LuaRuleEngineFunctions::LuaGetStartTime);
   ASSERT_EQ(registered["GetEndTime"], LuaRuleEngineFunctions::LuaGetEndTime);
   ASSERT_EQ(registered["SetDeltaTime"], LuaRuleEngineFunctions::LuaSetDeltaTime);
   ASSERT_EQ(registered["GetDeltaTime"], LuaRuleEngineFunctions::LuaGetDeltaTime);
   ASSERT_EQ(registered["SendInterFlow"], LuaRuleEngineFunctions::LuaSendInterFlow);
   ASSERT_EQ(registered["SendFinalFlow"], LuaRuleEngineFunctions::LuaSendFinalFlow);
   ASSERT_EQ(registered["SendAccStat"], LuaRuleEngineFunctions::LuaSendAccStat);
   ASSERT_EQ(registered["GetThreadId"], LuaRuleEngineFunctions::LuaGetThreadId);
   ASSERT_EQ(registered["GetRawMsgSize"], LuaRuleEngineFunctions::LuaGetRawMsgSize);
   ASSERT_EQ(registered["GetDpiMsgSize"], LuaRuleEngineFunctions::LuaGetDpiMsgSize);
   ASSERT_EQ(registered["SendStat"], LuaRuleEngineFunctions::LuaSendStat);

   delete functions;
}
using namespace networkMonitor;
using namespace std;

TEST_F(LuaFunctionsTest, StaticCallLuaGetDpiMsgSize) {
   DpiMsgLR dpiMsg;

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);

   string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
   dpiMsg.set_uuid(testUuid.c_str());

   string testEthSrc("00:22:19:08:2c:00");
   vector<unsigned char> ethSrc;
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x22);
   ethSrc.push_back(0x19);
   ethSrc.push_back(0x08);
   ethSrc.push_back(0x2c);
   ethSrc.push_back(0x00);
   dpiMsg.SetEthSrc(ethSrc);

   string testEthDst("f0:f7:55:dc:a8:00");

   vector<unsigned char> ethDst;
   ethDst.push_back(0xf0);
   ethDst.push_back(0xf7);
   ethDst.push_back(0x55);
   ethDst.push_back(0xdc);
   ethDst.push_back(0xa8);
   ethDst.push_back(0x00);
   dpiMsg.SetEthDst(ethDst);

   string testIpSrc = "10.1.10.50";
   uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
   dpiMsg.set_ipsrc(ipSrc);

   string testIpDst = "10.128.64.251";
   uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
   dpiMsg.set_ipdst(ipDst);

   string path("base.eth.ip.udp.ntp");
   dpiMsg.set_pktpath(path.c_str());

   string testIpSourcePort = "=12345"; // bogus, but easier to test
   dpiMsg.set_sourceport(12345);

   string testIpDestPort = "=54321"; // bogus, but easier to test
   dpiMsg.set_destport(54321);
   dpiMsg.set_protoid(12);
   dpiMsg.set_application_id_endq_proto_base(13);
   dpiMsg.add_application_endq_proto_base("wrong");
   dpiMsg.add_application_endq_proto_base("dummy");
   dpiMsg.set_sessionlenserver(12345);
   dpiMsg.set_deltasessionlenserver(12345);
   dpiMsg.set_sessionlenclient(6789);
   dpiMsg.set_deltasessionlenclient(6789);
   dpiMsg.set_packetcount(99);
   dpiMsg.set_deltapackets(99);
   dpiMsg.add_loginq_proto_aim("aLogin");
   dpiMsg.add_domainq_proto_smb("aDomain");
   dpiMsg.add_uri_fullq_proto_http("this/url.htm");
   dpiMsg.add_uriq_proto_http("notitUrl");
   dpiMsg.add_serverq_proto_http("thisname");
   dpiMsg.add_referer_serverq_proto_http("notitServer");
   dpiMsg.add_methodq_proto_ftp("TEST");
   dpiMsg.add_methodq_proto_ftp("COMMAND");
   dpiMsg.add_senderq_proto_smtp("test1");
   dpiMsg.add_receiverq_proto_smtp("test2");
   dpiMsg.add_subjectq_proto_smtp("test3");
   dpiMsg.add_versionq_proto_http("4.0");
   dpiMsg.add_filenameq_proto_gnutella("aFilename");
   dpiMsg.add_filename_encodingq_proto_aim_transfer("notitFile");
   dpiMsg.add_directoryq_proto_smb("aPath");
   dpiMsg.set_starttime(123);
   dpiMsg.set_endtime(456);
   dpiMsg.set_deltatime(333);
   dpiMsg.set_sessionidq_proto_ymsg(2345);
   int expectedSpaceUsed = dpiMsg.SpaceUsed();
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetDpiMsgSize(luaState);
   EXPECT_EQ(expectedSpaceUsed, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaIsIntermediateFlow) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect false
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaIsIntermediateFlow(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to FINAL, expect false
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaIsIntermediateFlow(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to INTERMEDIATE, expect true
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaIsIntermediateFlow(luaState);
   EXPECT_TRUE(lua_toboolean(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaIsIntermediateFinalFlow) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect false
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaIsIntermediateFinalFlow(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to FINAL, expect false
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaIsIntermediateFinalFlow(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to INTERMEDIATE_FINAL, expect true
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaIsIntermediateFinalFlow(luaState);
   EXPECT_TRUE(lua_toboolean(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetUuid) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, UUID is required field, initialized to ""
   std::string unknownUuid(UNKNOWN_UUID);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetUuid(luaState);
   EXPECT_EQ(unknownUuid, lua_tostring(luaState, -1));

   // Expect known value when set
   std::string knownUuid("5a36f34b-d8e0-47d4-8712-1daccda18c48");
   dpiMsg.set_uuid(knownUuid);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetUuid(luaState);
   EXPECT_EQ(knownUuid, lua_tostring(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetPacketCount) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetPacketCount(luaState);
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedPactetCount(236);
   dpiMsg.set_packetcount(expectedPactetCount);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetPacketCount(luaState);
   EXPECT_EQ(expectedPactetCount, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaSetDeltaPackets) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaPackets(221);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaPackets);
   LuaRuleEngineFunctions::LuaSetDeltaPackets(luaState);
   EXPECT_EQ(expectedDeltaPackets, dpiMsg.deltapackets());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetSessionLenServer) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetSessionLenServer(luaState);
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedSessionLenServer(99425);
   dpiMsg.set_sessionlenserver(expectedSessionLenServer);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetSessionLenServer(luaState);
   EXPECT_EQ(expectedSessionLenServer, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaSetDeltaSessionLenServer) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaSessionLenServer(10254);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaSessionLenServer);
   LuaRuleEngineFunctions::LuaSetDeltaSessionLenServer(luaState);
   EXPECT_EQ(expectedDeltaSessionLenServer, dpiMsg.deltasessionlenserver());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetSessionLenClient) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetSessionLenClient(luaState);
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedSessionLenClient(21553);
   dpiMsg.set_sessionlenclient(expectedSessionLenClient);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetSessionLenClient(luaState);
   EXPECT_EQ(expectedSessionLenClient, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaSetDeltaSessionLenClient) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaSessionLenClient(4521);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaSessionLenClient);
   LuaRuleEngineFunctions::LuaSetDeltaSessionLenClient(luaState);
   EXPECT_EQ(expectedDeltaSessionLenClient, dpiMsg.deltasessionlenclient());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetLatestApplication) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   dpiMsg.add_application_endq_proto_base("tcp");
   dpiMsg.add_application_endq_proto_base("http");
   dpiMsg.add_application_endq_proto_base("google");

   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetLatestApplication(luaState);
   std::string result = lua_tostring(luaState, -1);
   LOG(DEBUG) << "EXP: google got: " << result;
   EXPECT_EQ("google", result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetStartTime) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetStartTime(luaState);
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedStartTime(1367606483);
   dpiMsg.set_starttime(expectedStartTime);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetStartTime(luaState);
   EXPECT_EQ(expectedStartTime, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetEndTime) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetEndTime(luaState);
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedEndTime(1367606583);
   dpiMsg.set_endtime(expectedEndTime);
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaRuleEngineFunctions::LuaGetEndTime(luaState);
   EXPECT_EQ(expectedEndTime, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaSetDeltaTime) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaTime(632);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaTime);
   LuaRuleEngineFunctions::LuaSetDeltaTime(luaState);
   EXPECT_EQ(expectedDeltaTime, dpiMsg.deltatime());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallLuaSendInterFlow) {
#ifdef LR_DEBUG
   MockRuleEngine mRuleEngine(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, 0);
   mRuleEngine.mSiemMode = true;
   mRuleEngine.mSiemDebugMode = false;

   DpiMsgLR dpiMsg;

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE);

   string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
   dpiMsg.set_uuid(testUuid.c_str());

   string testEthSrc("00:22:19:08:2c:00");
   vector<unsigned char> ethSrc;
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x22);
   ethSrc.push_back(0x19);
   ethSrc.push_back(0x08);
   ethSrc.push_back(0x2c);
   ethSrc.push_back(0x00);
   dpiMsg.SetEthSrc(ethSrc);

   string testEthDst("f0:f7:55:dc:a8:00");

   vector<unsigned char> ethDst;
   ethDst.push_back(0xf0);
   ethDst.push_back(0xf7);
   ethDst.push_back(0x55);
   ethDst.push_back(0xdc);
   ethDst.push_back(0xa8);
   ethDst.push_back(0x00);
   dpiMsg.SetEthDst(ethDst);

   string testIpSrc = "10.1.10.50";
   uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
   dpiMsg.set_ipsrc(ipSrc);

   string testIpDst = "10.128.64.251";
   uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
   dpiMsg.set_ipdst(ipDst);

   string path("base.eth.ip.udp.ntp");
   dpiMsg.set_pktpath(path.c_str());

   string testIpSourcePort = "=12345"; // bogus, but easier to test
   dpiMsg.set_sourceport(12345);

   string testIpDestPort = "=54321"; // bogus, but easier to test
   dpiMsg.set_destport(54321);
   dpiMsg.set_protoid(12);
   dpiMsg.set_application_id_endq_proto_base(13);
   dpiMsg.add_application_endq_proto_base("wrong");
   dpiMsg.add_application_endq_proto_base("dummy");
   dpiMsg.set_sessionlenserver(12345);
   dpiMsg.set_sessionlenclient(6789);
   dpiMsg.set_packetcount(99);
   dpiMsg.add_loginq_proto_aim("aLogin");
   dpiMsg.add_domainq_proto_smb("aDomain");
   dpiMsg.add_uri_fullq_proto_http("this/url.htm");
   dpiMsg.add_uriq_proto_http("notitUrl");
   dpiMsg.add_serverq_proto_http("thisname");
   dpiMsg.add_referer_serverq_proto_http("notitServer");
   dpiMsg.add_methodq_proto_ftp("TEST|COMMAND");
   dpiMsg.add_senderq_proto_smtp("test1");
   dpiMsg.add_receiverq_proto_smtp("test2");
   dpiMsg.add_subjectq_proto_smtp("test3");
   dpiMsg.add_versionq_proto_http("4.0");
   dpiMsg.add_filenameq_proto_gnutella("aFilename");
   dpiMsg.add_filename_encodingq_proto_aim_transfer("notitFile");
   dpiMsg.add_directoryq_proto_smb("aPath");
   dpiMsg.set_starttime(123);
   dpiMsg.set_endtime(456);
   dpiMsg.set_deltatime(222);
   dpiMsg.set_deltasessionlenclient(567);
   dpiMsg.set_deltasessionlenserver(234);
   dpiMsg.set_deltapackets(33);
   dpiMsg.set_sessionidq_proto_ymsg(2345);
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushlightuserdata(luaState, &mRuleEngine);
   LuaRuleEngineFunctions::LuaSendInterFlow(luaState);

   dpiMsg.set_endtime(567);
   dpiMsg.set_sessionlenserver(23456);
   dpiMsg.set_sessionlenclient(7890);
   dpiMsg.set_packetcount(124);
   dpiMsg.set_deltatime(111); // 567 - 456
   dpiMsg.set_deltasessionlenclient(1101); // 7890 - 6789
   dpiMsg.set_deltasessionlenserver(11111); // 23456 - 12345
   dpiMsg.set_deltapackets(25); // 124 - 99

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushlightuserdata(luaState, &mRuleEngine);
   LuaRuleEngineFunctions::LuaSendInterFlow(luaState);

   //std::cout << "SyslogOutput: " << sysLogOutput << std::endl;
   // Did the data show up in the syslog output
   ASSERT_EQ(2, sysLogOutput.size());
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("EVT:003 "));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find(testUuid));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,dummy,567/6789,234/12345,33/99,123,456,222/333"));
   EXPECT_NE(std::string::npos, sysLogOutput[1].find("EVT:003 "));
   EXPECT_NE(std::string::npos, sysLogOutput[1].find(testUuid));
   EXPECT_NE(std::string::npos, sysLogOutput[1].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,dummy,1101/7890,11111/23456,25/124,123,567,111/444"));
   lua_close(luaState);
#endif
}

TEST_F(LuaFunctionsTest, StaticCallLuaSendFinalFlow) {
#ifdef LR_DEBUG
   MockRuleEngine mRuleEngine(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, 0);
   mRuleEngine.mSiemMode = true;
   mRuleEngine.mSiemDebugMode = false;

   DpiMsgLR dpiMsg;

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);

   string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
   dpiMsg.set_uuid(testUuid.c_str());

   string testEthSrc("00:22:19:08:2c:00");
   vector<unsigned char> ethSrc;
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x22);
   ethSrc.push_back(0x19);
   ethSrc.push_back(0x08);
   ethSrc.push_back(0x2c);
   ethSrc.push_back(0x00);
   dpiMsg.SetEthSrc(ethSrc);

   string testEthDst("f0:f7:55:dc:a8:00");

   vector<unsigned char> ethDst;
   ethDst.push_back(0xf0);
   ethDst.push_back(0xf7);
   ethDst.push_back(0x55);
   ethDst.push_back(0xdc);
   ethDst.push_back(0xa8);
   ethDst.push_back(0x00);
   dpiMsg.SetEthDst(ethDst);

   string testIpSrc = "10.1.10.50";
   uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
   dpiMsg.set_ipsrc(ipSrc);

   string testIpDst = "10.128.64.251";
   uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
   dpiMsg.set_ipdst(ipDst);

   string path("base.eth.ip.udp.ntp");
   dpiMsg.set_pktpath(path.c_str());

   string testIpSourcePort = "=12345"; // bogus, but easier to test
   dpiMsg.set_sourceport(12345);

   string testIpDestPort = "=54321"; // bogus, but easier to test
   dpiMsg.set_destport(54321);
   dpiMsg.set_protoid(12);
   dpiMsg.set_application_id_endq_proto_base(13);
   dpiMsg.add_application_endq_proto_base("wrong");
   dpiMsg.add_application_endq_proto_base("dummy");
   dpiMsg.set_sessionlenserver(12345);
   dpiMsg.set_deltasessionlenserver(12345);
   dpiMsg.set_sessionlenclient(6789);
   dpiMsg.set_deltasessionlenclient(6789);
   dpiMsg.set_packetcount(99);
   dpiMsg.set_deltapackets(99);
   dpiMsg.add_loginq_proto_aim("aLogin");
   dpiMsg.add_domainq_proto_smb("aDomain");
   dpiMsg.add_uri_fullq_proto_http("this/url.htm");
   dpiMsg.add_uriq_proto_http("notitUrl");
   dpiMsg.add_serverq_proto_http("thisname");
   dpiMsg.add_referer_serverq_proto_http("notitServer");
   dpiMsg.add_methodq_proto_ftp("TEST");
   dpiMsg.add_methodq_proto_ftp("COMMAND");
   dpiMsg.add_senderq_proto_smtp("test1");
   dpiMsg.add_receiverq_proto_smtp("test2");
   dpiMsg.add_subjectq_proto_smtp("test3");
   dpiMsg.add_versionq_proto_http("4.0");
   dpiMsg.add_filenameq_proto_gnutella("aFilename");
   dpiMsg.add_filename_encodingq_proto_aim_transfer("notitFile");
   dpiMsg.add_directoryq_proto_smb("aPath");
   dpiMsg.set_starttime(123);
   dpiMsg.set_endtime(456);
   dpiMsg.set_deltatime(333);
   dpiMsg.set_sessionidq_proto_ymsg(2345);

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushlightuserdata(luaState, &mRuleEngine);
   LuaRuleEngineFunctions::LuaSendFinalFlow(luaState);

   //std::cout << "SyslogOutput: " << sysLogOutput << std::endl;
   // Did the data show up in the syslog output
   ASSERT_EQ(1, sysLogOutput.size());
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("EVT:001 "));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find(testUuid));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,dummy,6789/6789,12345/12345,99/99,123,456,333/333"));
   lua_close(luaState);
#endif
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetThreadId) {
#ifdef LR_DEBUG
   unsigned int expectedThreadId(5);
   MockRuleEngine mre(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, expectedThreadId);
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect the threadId to be 5 as initialized.
   lua_pushlightuserdata(luaState, &mre);
   LuaRuleEngineFunctions::LuaGetThreadId(luaState);
   EXPECT_EQ(expectedThreadId, lua_tointeger(luaState, -1));
   lua_close(luaState);
#endif
}

TEST_F(LuaFunctionsTest, StaticCallLuaGetRawMsgSize) {
#ifdef LR_DEBUG
   MockRuleEngine mre(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, 0);
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect the raw message size to match the initialized value of 0
   lua_pushlightuserdata(luaState, &mre);
   LuaRuleEngineFunctions::LuaGetRawMsgSize(luaState);
   EXPECT_EQ(0, lua_tointeger(luaState, -1));
   lua_close(luaState);
#endif
}