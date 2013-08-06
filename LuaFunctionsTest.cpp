
#include <stdlib.h>
#include "LuaFunctionsTest.h"
#include "MockConf.h"
#include "g2log.hpp"
#include "MockLuaExecuter.h"
#include "LuaPacketFunctions.h"
#include "LuaRuleEngineFunctions.h"
#include "RuleEngine.h"
#include "MockRuleEngine.h"
#include "MockDpiMsgLR.h"

static int LuaTestFunction(lua_State *L) {
   return 0;
}

static int LuaTestFunction2(lua_State *L) {
   return 0;
}

TEST_F(LuaFunctionsTest, BasicFunctions) {
   LuaFunctions* functions = new LuaFunctions;

   MockLuaExecuter ruleEngine;
   functions->RegisterAllKnownFunctions(ruleEngine);

   auto registered = ruleEngine.GetPossibleFunctions();

   ASSERT_TRUE(registered.end() != registered.find("GetFullStringFromFlow"));
   ASSERT_EQ(registered["GetFullStringFromFlow"], LuaFunctions::GetFullStringFromDpi);
   ASSERT_TRUE(registered.end() != registered.find("GetStringsInFlow"));
   ASSERT_EQ(registered["GetStringsInFlow"], LuaFunctions::GetListOfStrings);
   ASSERT_TRUE(registered.end() != registered.find("GetIntFromFlow"));
   ASSERT_EQ(registered["GetIntFromFlow"], LuaFunctions::GetIntFromDpi);
   ASSERT_TRUE(registered.end() != registered.find("GetIntsInFlow"));
   ASSERT_EQ(registered["GetIntsInFlow"], LuaFunctions::GetListOfInts);
   ASSERT_TRUE(registered.end() != registered.find("GetLongFromFlow"));
   ASSERT_EQ(registered["GetLongFromFlow"], LuaFunctions::GetLongFromDpi);
   ASSERT_TRUE(registered.end() != registered.find("GetLongsInFlow"));
   ASSERT_EQ(registered["GetLongsInFlow"], LuaFunctions::GetListOfLongs);
   ASSERT_TRUE(registered.end() != registered.find("GetLatestStringFromFlow"));
   ASSERT_EQ(registered["GetLatestStringFromFlow"], LuaFunctions::GetLatestStringFromDpi);
   ASSERT_TRUE(registered.end() != registered.find("GetSourceIPFromFlow"));
   ASSERT_EQ(registered["GetSourceIPFromFlow"], LuaFunctions::GetSourceIPFromDpi);
   ASSERT_TRUE(registered.end() != registered.find("GetDestIPFromFlow"));
   ASSERT_EQ(registered["GetDestIPFromFlow"], LuaFunctions::GetDestIPFromDpi);
   ASSERT_TRUE(registered.end() != registered.find("GetSourceMACFromFlow"));
   ASSERT_EQ(registered["GetSourceMACFromFlow"], LuaFunctions::GetSourceMACFromDpi);
   ASSERT_TRUE(registered.end() != registered.find("GetDestMACFromFlow"));
   ASSERT_EQ(registered["GetDestMACFromFlow"], LuaFunctions::GetDestMACFromDpi);

   ASSERT_TRUE(registered.end() != registered.find("GetDpiMsgSize"));

   ASSERT_EQ(registered["GetDpiMsgSize"], LuaFunctions::GetDpiMsgSize);

   ASSERT_TRUE(registered.end() != registered.find("GetUuid"));
   ASSERT_TRUE(registered.end() != registered.find("IsIntermediateFlow"));
   ASSERT_TRUE(registered.end() != registered.find("IsIntermediateFinalFlow"));
   ASSERT_TRUE(registered.end() != registered.find("IsFinalFlow"));

   ASSERT_TRUE(registered.end() != registered.find("GetLatestApplication"));
   ASSERT_TRUE(registered.end() != registered.find("SetCustomApplication"));
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
   ASSERT_TRUE(registered.end() != registered.find("SetFlowState"));
   ASSERT_TRUE(registered.end() != registered.find("DebugLog"));
   ASSERT_TRUE(registered.end() != registered.find("WarningLog"));
   ASSERT_TRUE(registered.end() != registered.find("SetChildFlowNum"));

   ASSERT_EQ(registered["GetLatestApplication"], LuaRuleEngineFunctions::GetLatestApplication);
   ASSERT_EQ(registered["SetCustomApplication"], LuaRuleEngineFunctions::SetCustomApplication);
   ASSERT_EQ(registered["GetPacketCount"], LuaRuleEngineFunctions::GetPacketCount);
   ASSERT_EQ(registered["SetDeltaPackets"], LuaRuleEngineFunctions::SetDeltaPackets);
   ASSERT_EQ(registered["GetSessionLenServer"], LuaRuleEngineFunctions::GetSessionLenServer);
   ASSERT_EQ(registered["SetDeltaSessionLenServer"], LuaRuleEngineFunctions::SetDeltaSessionLenServer);
   ASSERT_EQ(registered["GetSessionLenClient"], LuaRuleEngineFunctions::GetSessionLenClient);
   ASSERT_EQ(registered["SetDeltaSessionLenClient"], LuaRuleEngineFunctions::SetDeltaSessionLenClient);
   ASSERT_EQ(registered["GetStartTime"], LuaRuleEngineFunctions::GetStartTime);
   ASSERT_EQ(registered["GetEndTime"], LuaRuleEngineFunctions::GetEndTime);
   ASSERT_EQ(registered["SetDeltaTime"], LuaRuleEngineFunctions::SetDeltaTime);
   ASSERT_EQ(registered["GetDeltaTime"], LuaRuleEngineFunctions::GetDeltaTime);
   ASSERT_EQ(registered["GetUuid"], LuaFunctions::GetSessionId);
   ASSERT_EQ(registered["SetFlowState"], LuaFunctions::SetFlowState);
   ASSERT_EQ(registered["DebugLog"], LuaFunctions::DebugLog);
   ASSERT_EQ(registered["WarningLog"], LuaFunctions::WarningLog);
   ASSERT_EQ(registered["SetChildFlowNum"], LuaRuleEngineFunctions::SetChildFlowNum);
   delete functions;
}

TEST_F(LuaFunctionsTest, AddRemoveFromContainer) {
   MockLuaFunctions functions;

   int number = functions.NumberOfFunctions();
   functions.AddFunction("Test1", LuaTestFunction);
   ASSERT_EQ(number + 1, functions.NumberOfFunctions());
   functions.AddFunction("Test1", LuaTestFunction2);
   ASSERT_EQ(number + 1, functions.NumberOfFunctions());
   functions.RemoveFunction("Test1");
   ASSERT_EQ(number, functions.NumberOfFunctions());
}

TEST_F(LuaFunctionsTest, LuaGetIpInfoFromDpi) {
   networkMonitor::DpiMsgLR dpiMsg;
   uint32_t ipDst = 0x4B00010A; // 10.1.0.75
   uint32_t ipSrc = 0x6401A8C0; // 192.168.1.100
   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   dpiMsg.set_ipdest(ipDst);
   dpiMsg.set_ipsource(ipSrc);

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaFunctions::GetSourceIPFromDpi(luaState);
   std::string result = lua_tostring(luaState, -1);
   EXPECT_EQ("192.168.1.100", result);
   lua_close(luaState);

   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaFunctions::GetDestIPFromDpi(luaState);
   result = lua_tostring(luaState, -1);
   EXPECT_EQ("10.1.0.75", result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, LuaGetMACInfoFromDpi) {
   networkMonitor::DpiMsgLR dpiMsg;
   uint32_t ipDst = 0x4B00010A; // 10.1.0.75
   uint32_t ipSrc = 0x6401A8C0; // 192.168.1.100
   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   dpiMsg.set_ipdest(ipDst);
   dpiMsg.set_ipsource(ipSrc);
   vector<unsigned char> ethSrc;
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x50);
   ethSrc.push_back(0x56);
   ethSrc.push_back(0xBE);
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x1C);
   dpiMsg.SetEthSrc(ethSrc);
   vector<unsigned char> ethDst;
   ethDst.push_back(0x10);
   ethDst.push_back(0x52);
   ethDst.push_back(0x36);
   ethDst.push_back(0xB4);
   ethDst.push_back(0x50);
   ethDst.push_back(0x16);
   dpiMsg.SetEthDst(ethDst);
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaFunctions::GetDestMACFromDpi(luaState);
   std::string result = lua_tostring(luaState, -1);
   EXPECT_EQ("10:52:36:b4:50:16", result);
   lua_close(luaState);

   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaFunctions::GetSourceMACFromDpi(luaState);
   result = lua_tostring(luaState, -1);
   EXPECT_EQ("00:50:56:be:00:1c", result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, GetLatestStringFromDpi) {
   networkMonitor::DpiMsgLR dpiMsg;

   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushstring(luaState, "accept_encodingq_proto_http");
   LuaFunctions::GetLatestStringFromDpi(luaState);
   std::string result = lua_tostring(luaState, -1);
   EXPECT_EQ("test2", result);
   lua_close(luaState);

   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushstring(luaState, "sessionid");
   EXPECT_EQ(1, LuaFunctions::GetLatestStringFromDpi(luaState));
   result = lua_tostring(luaState, -1);
   EXPECT_EQ("uuid", result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, LuaGetFullListFromDpi) {
   networkMonitor::DpiMsgLR dpiMsg;

   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushstring(luaState, "accept_encodingq_proto_http");
   LuaFunctions::GetFullStringFromDpi(luaState);
   std::string result = lua_tostring(luaState, -1);
   EXPECT_EQ("test1|test2", result);
   lua_close(luaState);

   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushstring(luaState, "sessionid");
   EXPECT_EQ(1, LuaFunctions::GetFullStringFromDpi(luaState));
   result = lua_tostring(luaState, -1);
   EXPECT_EQ("uuid", result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, GetListOfInts) {
   networkMonitor::DpiMsgLR dpiMsg;

   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   dpiMsg.set_ack_numberq_proto_tcp(1234);
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   EXPECT_EQ(1, LuaFunctions::GetListOfInts(luaState));
   std::vector<std::string> fields;
   for (int i = 1; i < lua_gettop(luaState); i++) {
      ASSERT_EQ(LUA_TSTRING, lua_type(luaState, i + 1));
      fields.push_back(lua_tostring(luaState, i + 1));
   }
   ASSERT_EQ(1, fields.size());
   EXPECT_NE(fields.end(), std::find(fields.begin(), fields.end(), "ack_numberq_proto_tcp"));
   lua_close(luaState);

}

TEST_F(LuaFunctionsTest, GetIntFromDpi) {
   networkMonitor::DpiMsgLR dpiMsg;

   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   dpiMsg.set_ack_numberq_proto_tcp(1234);
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushstring(luaState, "ack_numberq_proto_tcp");
   LuaFunctions::GetIntFromDpi(luaState);
   int result = lua_tointeger(luaState, -1);
   EXPECT_EQ(1234, result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, GetListOfLongs) {
   networkMonitor::DpiMsgLR dpiMsg;

   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   dpiMsg.set_ack_numberq_proto_tcp(1234);
   dpiMsg.set_avp_int64q_proto_radius(123456789L);
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   EXPECT_EQ(1, LuaFunctions::GetListOfLongs(luaState));
   std::vector<std::string> fields;
   for (int i = 1; i < lua_gettop(luaState); i++) {
      ASSERT_EQ(LUA_TSTRING, lua_type(luaState, i + 1));
      fields.push_back(lua_tostring(luaState, i + 1));
   }
   ASSERT_EQ(1, fields.size());
   EXPECT_NE(fields.end(), std::find(fields.begin(), fields.end(), "avp_int64q_proto_radius"));
   lua_close(luaState);

}

TEST_F(LuaFunctionsTest, GetLongFromDpi) {
   networkMonitor::DpiMsgLR dpiMsg;

   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   dpiMsg.set_ack_numberq_proto_tcp(1234);
   dpiMsg.set_avp_int64q_proto_radius(123456789L);
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushstring(luaState, "avp_int64q_proto_radius");
   LuaFunctions::GetLongFromDpi(luaState);
   int64_t result = lua_tointeger(luaState, -1);
   EXPECT_EQ(123456789L, result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, GetListOfStrings) {
   networkMonitor::DpiMsgLR dpiMsg;

   dpiMsg.add_accept_encodingq_proto_http("test1");
   dpiMsg.add_accept_encodingq_proto_http("test2");
   dpiMsg.set_sessionid("uuid");
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   EXPECT_EQ(2, LuaFunctions::GetListOfStrings(luaState));
   std::vector<std::string> fields;
   for (int i = 1; i < lua_gettop(luaState); i++) {
      ASSERT_EQ(LUA_TSTRING, lua_type(luaState, i + 1));
      fields.push_back(lua_tostring(luaState, i + 1));
   }
   ASSERT_EQ(2, fields.size());
   EXPECT_NE(fields.end(), std::find(fields.begin(), fields.end(), "sessionid"));
   EXPECT_NE(fields.end(), std::find(fields.begin(), fields.end(), "accept_encodingq_proto_http"));
   lua_close(luaState);

}
#ifdef LR_DEUBG

TEST_F(LuaFunctionsTest, AddThenRegister) {
   MockLuaFunctions functions;

   int number = functions.NumberOfFunctions();
   functions.AddFunction("Test1", LuaTestFunction);
   ASSERT_EQ(number + 1, functions.NumberOfFunctions());
   functions.AddFunction("Test2", LuaTestFunction2);
   ASSERT_EQ(number + 2, functions.NumberOfFunctions());

   MockLuaExecuter ruleEngine;
   functions.RegisterAllKnownFunctions(ruleEngine);

   auto registered = ruleEngine.GetPossibleFunctions();

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

   auto registered = ruleEngine.GetPossibleFunctions();
   ASSERT_TRUE(registered.end() != registered.find("SessionAge"));
   ASSERT_TRUE(registered.end() != registered.find("EndOfFlow"));
   ASSERT_TRUE(registered.end() != registered.find("GetCurrentClassification"));
   ASSERT_TRUE(registered.end() != registered.find("CapturePacket"));
   ASSERT_TRUE(registered.end() != registered.find("WriteAllCapturedPackets"));
   ASSERT_TRUE(registered.end() != registered.find("CleanupCapturedPackets"));
   ASSERT_TRUE(registered.end() != registered.find("GetPacketBytes"));
   ASSERT_TRUE(registered.end() != registered.find("SetMessageDebug"));
   ASSERT_EQ(registered["SetMessageDebug"], LuaPacketFunctions::SetMessageDebug);

   ASSERT_EQ(registered["SessionAge"], LuaPacketFunctions::SessionAge);
   ASSERT_EQ(registered["EndOfFlow"], LuaPacketFunctions::DataNull);
   ASSERT_EQ(registered["GetCurrentClassification"], LuaPacketFunctions::DpiMsgClassification);
   ASSERT_EQ(registered["CapturePacket"], LuaPacketFunctions::BufferSessionToMemory);
   ASSERT_EQ(registered["WriteAllCapturedPackets"], LuaPacketFunctions::WriteAllPacketsToDisk);
   ASSERT_EQ(registered["CleanupCapturedPackets"], LuaPacketFunctions::EmptyBufferOfSession);
   ASSERT_EQ(registered["GetPacketBytes"], LuaPacketFunctions::GetPacketBytes);

   delete functions;
}

#endif
TEST_F(LuaFunctionsTest, SessionAge) {
   networkMonitor::DpiMsgLR dpiMsg;

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, 10);
   LuaPacketFunctions::SessionAge(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));
   lua_close(luaState);
   luaState = luaL_newstate();
   dpiMsg.set_timeupdate(std::time(NULL));
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, 999);
   LuaPacketFunctions::SessionAge(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));
   lua_close(luaState);
   luaState = luaL_newstate();
   time_t pasttime = std::time(NULL) - 11;
   dpiMsg.set_timeupdate(pasttime);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, 10);
   LuaPacketFunctions::SessionAge(luaState);
   EXPECT_TRUE(lua_toboolean(luaState, -1));
   EXPECT_TRUE(dpiMsg.flowtype() == ::networkMonitor::DpiMsgLRproto_Type_INTERMEDIATE);
   EXPECT_NE(pasttime, dpiMsg.timeupdate());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, DataNull) {
   networkMonitor::DpiMsgLR dpiMsg;

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::DataNull(luaState);
   EXPECT_FALSE(lua_toboolean(luaState, -1));
   lua_close(luaState);
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, NULL);
   LuaPacketFunctions::DataNull(luaState);
   EXPECT_TRUE(lua_toboolean(luaState, -1));

   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, DpiMsgClassification) {
   networkMonitor::DpiMsgLR dpiMsg;

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::DpiMsgClassification(luaState);
   std::string classification(lua_tostring(luaState, -1));
   EXPECT_EQ(EMPTY, classification);
   lua_close(luaState);
   luaState = luaL_newstate();
   dpiMsg.add_applicationq_proto_base("test1");
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::DpiMsgClassification(luaState);
   classification = lua_tostring(luaState, -1);
   EXPECT_EQ("test1", classification);
   lua_close(luaState);
   dpiMsg.add_application_endq_proto_base("test2");
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   LuaPacketFunctions::DpiMsgClassification(luaState);
   classification = lua_tostring(luaState, -1);
   EXPECT_EQ("test2", classification);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, GetPacketBytes) {
   struct upacket packet;
   unsigned char byteData[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

   packet.buffer = byteData;
   packet.len = 10;
   int first = 5;
   int last = 5;

   // Check one byte
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(5, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Check four bytes
   luaState = luaL_newstate();
   first = 6;
   last = 9;
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(0x06070809, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Get first byte from packet
   luaState = luaL_newstate();
   first = 1;
   last = 1;
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(1, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Get last byte from packet
   luaState = luaL_newstate();
   first = 10;
   last = 10;
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(10, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Check first equal to zero
   luaState = luaL_newstate();
   first = 0;
   last = 0;
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(0, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Check first greater than last
   luaState = luaL_newstate();
   first = 7;
   last = 6;
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(0, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Check greater than four bytes
   luaState = luaL_newstate();
   first = 3;
   last = 8;
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(0, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Check past end of byte array
   luaState = luaL_newstate();
   first = 10;
   last = 11;
   lua_pushlightuserdata(luaState, &packet);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(0, lua_tonumber(luaState, -1));
   lua_close(luaState);

   // Check null packet
   luaState = luaL_newstate();
   first = 3;
   last = 4;
   lua_pushlightuserdata(luaState, NULL);
   lua_pushinteger(luaState, first);
   lua_pushinteger(luaState, last);
   ASSERT_EQ(1, LuaPacketFunctions::GetPacketBytes(luaState));
   EXPECT_EQ(0, lua_tonumber(luaState, -1));
   lua_close(luaState);
}

#ifdef LR_DEBUG

TEST_F(LuaFunctionsTest, RuleEngineFunctions) {
   LuaRuleEngineFunctions* functions = new LuaRuleEngineFunctions;
   MockConf conf;

   MockLuaExecuter ruleEngine;
   functions->RegisterAllKnownFunctions(ruleEngine);

   auto registered = ruleEngine.GetPossibleFunctions();


   ASSERT_TRUE(registered.end() != registered.find("SendInterFlowToSyslog"));
   ASSERT_TRUE(registered.end() != registered.find("SendFinalFlowToSyslog"));
   ASSERT_TRUE(registered.end() != registered.find("CleanInterFlow"));
   ASSERT_TRUE(registered.end() != registered.find("SendAccStat"));
   ASSERT_TRUE(registered.end() != registered.find("GetThreadId"));
   ASSERT_TRUE(registered.end() != registered.find("SendFlowToSearch"));

   ASSERT_TRUE(registered.end() != registered.find("SendStat"));
   ASSERT_EQ(registered["IsIntermediateFlow"], LuaRuleEngineFunctions::IsIntermediateFlow);
   ASSERT_EQ(registered["IsIntermediateFinalFlow"], LuaRuleEngineFunctions::IsIntermediateFinalFlow);
   ASSERT_EQ(registered["IsFinalFlow"], LuaRuleEngineFunctions::IsFinalFlow);
   ASSERT_TRUE(registered.end() != registered.find("GetRawMsgSize"));
   ASSERT_EQ(registered["GetRawMsgSize"], LuaRuleEngineFunctions::GetRawMsgSize);

   ASSERT_EQ(registered["SendInterFlowToSyslog"], LuaRuleEngineFunctions::SendInterFlowToSyslog);
   ASSERT_EQ(registered["SendFinalFlowToSyslog"], LuaRuleEngineFunctions::SendFinalFlowToSyslog);
   ASSERT_EQ(registered["CleanInterFlow"], LuaRuleEngineFunctions::CleanInterFlow);
   ASSERT_EQ(registered["SendAccStat"], LuaRuleEngineFunctions::SendAccStat);
   ASSERT_EQ(registered["GetThreadId"], LuaRuleEngineFunctions::GetThreadId);
   ASSERT_EQ(registered["SendFlowToSearch"], LuaRuleEngineFunctions::SendFlowToSearch);

   ASSERT_EQ(registered["SendStat"], LuaRuleEngineFunctions::SendStat);

   delete functions;
}
#endif
using namespace networkMonitor;
using namespace std;

TEST_F(LuaFunctionsTest, StaticCallGetDpiMsgSize) {
   DpiMsgLR dpiMsg;

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);

   string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
   dpiMsg.set_sessionid(testUuid.c_str());

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
   dpiMsg.set_ipsource(ipSrc);

   string testIpDst = "10.128.64.251";
   uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
   dpiMsg.set_ipdest(ipDst);

   string path("base.eth.ip.udp.ntp");
   dpiMsg.set_pktpath(path.c_str());

   string testIpSourcePort = "=12345"; // bogus, but easier to test
   dpiMsg.set_portsource(12345);

   string testIpDestPort = "=54321"; // bogus, but easier to test
   dpiMsg.set_portdest(54321);
   dpiMsg.set_protoid(12);
   dpiMsg.set_application_id_endq_proto_base(13);
   dpiMsg.add_application_endq_proto_base("wrong");
   dpiMsg.add_application_endq_proto_base("dummy");
   dpiMsg.set_bytesserver(12345);
   dpiMsg.set_bytesserverdelta(12345);
   dpiMsg.set_bytesclient(6789);
   dpiMsg.set_bytesclientdelta(6789);
   dpiMsg.set_packettotal(99);
   dpiMsg.set_packetsdelta(99);
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
   dpiMsg.set_timestart(123);
   dpiMsg.set_timeupdate(456);
   dpiMsg.set_timedelta(333);
   dpiMsg.set_sessionidq_proto_ymsg(2345);
   int expectedSpaceUsed = dpiMsg.SpaceUsed();
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetDpiMsgSize(luaState));
   EXPECT_EQ(expectedSpaceUsed, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, SetFlowState) {
   networkMonitor::DpiMsgLR dpiMsg;
   string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
   dpiMsg.set_sessionid(testUuid.c_str());

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, networkMonitor::DpiMsgLRproto_FlowStateType_MID_FLOW_DETECT);
   ASSERT_EQ(0, LuaPacketFunctions::SetFlowState(luaState));
   EXPECT_EQ(networkMonitor::DpiMsgLRproto_FlowStateType_MID_FLOW_DETECT, dpiMsg.flowstate());
   lua_close(luaState);

   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, networkMonitor::DpiMsgLRproto_FlowStateType_SYN);
   ASSERT_EQ(0, LuaPacketFunctions::SetFlowState(luaState));
   EXPECT_EQ(networkMonitor::DpiMsgLRproto_FlowStateType_SYN, dpiMsg.flowstate());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, DebugLog) {
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushstring(luaState, "test debug 1 2 3");
   LuaPacketFunctions::DebugLog(luaState);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, WarningLog) {
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushstring(luaState, "test warning 1 2 3");
   LuaPacketFunctions::WarningLog(luaState);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallSetChildFlowNum) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   EXPECT_FALSE(dpiMsg.has_childflownum());
   // Expect known value when set
   int expectedChildFlowNum(1);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedChildFlowNum);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetChildFlowNum(luaState));
   EXPECT_TRUE(dpiMsg.has_childflownum());
   EXPECT_EQ(expectedChildFlowNum, dpiMsg.childflownum());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallIsIntermediateFlow) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect false
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::IsIntermediateFlow(luaState));
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to FINAL, expect false
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::IsIntermediateFlow(luaState));
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to INTERMEDIATE, expect true
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::IsIntermediateFlow(luaState));
   EXPECT_TRUE(lua_toboolean(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallIsIntermediateFinalFlow) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect false
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::IsIntermediateFinalFlow(luaState));
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to FINAL, expect false
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::IsIntermediateFinalFlow(luaState));
   EXPECT_FALSE(lua_toboolean(luaState, -1));

   // Value set to INTERMEDIATE_FINAL, expect true
   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::IsIntermediateFinalFlow(luaState));
   EXPECT_TRUE(lua_toboolean(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallGetUuid) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, UUID is required field, initialized to ""
   std::string unknownUuid(UNKNOWN_SESSIONID);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetSessionId(luaState));
   EXPECT_EQ(unknownUuid, lua_tostring(luaState, -1));

   // Expect known value when set
   std::string knownUuid("5a36f34b-d8e0-47d4-8712-1daccda18c48");
   dpiMsg.set_sessionid(knownUuid);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetSessionId(luaState));
   EXPECT_EQ(knownUuid, lua_tostring(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallGetPacketCount) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetPacketCount(luaState));
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedPactetCount(236);
   dpiMsg.set_packettotal(expectedPactetCount);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetPacketCount(luaState));
   EXPECT_EQ(expectedPactetCount, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallSetDeltaPackets) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaPackets(221);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaPackets);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetDeltaPackets(luaState));
   EXPECT_EQ(expectedDeltaPackets, dpiMsg.packetsdelta());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallGetSessionLenServer) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetSessionLenServer(luaState));
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedSessionLenServer(99425);
   dpiMsg.set_bytesserver(expectedSessionLenServer);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetSessionLenServer(luaState));
   EXPECT_EQ(expectedSessionLenServer, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallSetDeltaSessionLenServer) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaSessionLenServer(10254);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaSessionLenServer);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetDeltaSessionLenServer(luaState));
   EXPECT_EQ(expectedDeltaSessionLenServer, dpiMsg.bytesserverdelta());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallGetSessionLenClient) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetSessionLenClient(luaState));
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedSessionLenClient(21553);
   dpiMsg.set_bytesclient(expectedSessionLenClient);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetSessionLenClient(luaState));
   EXPECT_EQ(expectedSessionLenClient, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallSetDeltaSessionLenClient) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaSessionLenClient(4521);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaSessionLenClient);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetDeltaSessionLenClient(luaState));
   EXPECT_EQ(expectedDeltaSessionLenClient, dpiMsg.bytesclientdelta());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallGetLatestApplication) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   dpiMsg.add_application_endq_proto_base("tcp");
   dpiMsg.add_application_endq_proto_base("http");
   dpiMsg.add_application_endq_proto_base("google");

   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetLatestApplication(luaState));
   std::string result = lua_tostring(luaState, -1);
   LOG(DEBUG) << "EXP: google got: " << result;
   EXPECT_EQ("google", result);
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallSetCustomApplication) {
   DpiMsgLR dpiMsg;
   lua_State *luaState1;
   luaState1 = luaL_newstate();
   lua_pushlightuserdata(luaState1, &dpiMsg);
   lua_pushstring(luaState1, "custom_app_1");
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetCustomApplication(luaState1));
   EXPECT_EQ(1, dpiMsg.customapplication_size());
   EXPECT_EQ("custom_app_1", dpiMsg.customapplication(0));
   lua_close(luaState1);

   lua_State *luaState2;
   luaState2 = luaL_newstate();
   lua_pushlightuserdata(luaState2, &dpiMsg);
   lua_pushstring(luaState2, "custom_app_1");
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetCustomApplication(luaState2));
   EXPECT_EQ(1, dpiMsg.customapplication_size());
   EXPECT_EQ("custom_app_1", dpiMsg.customapplication(0));
   lua_close(luaState2);

   lua_State *luaState3;
   luaState3 = luaL_newstate();
   lua_pushlightuserdata(luaState3, &dpiMsg);
   lua_pushstring(luaState3, "custom_app_2");
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetCustomApplication(luaState3));
   EXPECT_EQ(2, dpiMsg.customapplication_size());
   EXPECT_EQ("custom_app_1", dpiMsg.customapplication(0));
   EXPECT_EQ("custom_app_2", dpiMsg.customapplication(1));
   lua_close(luaState3);
}

TEST_F(LuaFunctionsTest, StaticCallGetStartTime) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetStartTime(luaState));
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedStartTime(1367606483);
   dpiMsg.set_timestart(expectedStartTime);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetStartTime(luaState));
   EXPECT_EQ(expectedStartTime, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallGetEndTime) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Value not set, expect 0
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetEndTime(luaState));
   EXPECT_EQ(0, lua_tointeger(luaState, -1));

   // Expect known value when set
   int expectedEndTime(1367606583);
   dpiMsg.set_timeupdate(expectedEndTime);
   lua_pushlightuserdata(luaState, &dpiMsg);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetEndTime(luaState));
   EXPECT_EQ(expectedEndTime, lua_tointeger(luaState, -1));
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallSetDeltaTime) {
   DpiMsgLR dpiMsg;
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect known value when set
   int expectedDeltaTime(632);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushinteger(luaState, expectedDeltaTime);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SetDeltaTime(luaState));
   EXPECT_EQ(expectedDeltaTime, dpiMsg.timedelta());
   lua_close(luaState);
}

TEST_F(LuaFunctionsTest, StaticCallSendInterFlow) {
#ifdef LR_DEBUG
   MockRuleEngine mRuleEngine(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, 0);
   mRuleEngine.mSiemMode = true;
   mRuleEngine.mSiemDebugMode = false;

   DpiMsgLR dpiMsg;

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE);

   string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
   dpiMsg.set_sessionid(testUuid.c_str());

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
   dpiMsg.set_ipsource(ipSrc);

   string testIpDst = "10.128.64.251";
   uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
   dpiMsg.set_ipdest(ipDst);

   string path("base.eth.ip.udp.ntp");
   dpiMsg.set_pktpath(path.c_str());

   string testIpSourcePort = "=12345"; // bogus, but easier to test
   dpiMsg.set_portsource(12345);

   string testIpDestPort = "=54321"; // bogus, but easier to test
   dpiMsg.set_portdest(54321);
   dpiMsg.set_protoid(12);
   dpiMsg.set_application_id_endq_proto_base(13);
   dpiMsg.add_application_endq_proto_base("wrong");
   dpiMsg.add_application_endq_proto_base("unknown");
   dpiMsg.set_bytesserver(12345);
   dpiMsg.set_bytesclient(6789);
   dpiMsg.set_packettotal(99);
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
   dpiMsg.set_timestart(123);
   dpiMsg.set_timeupdate(456);
   dpiMsg.set_timedelta(222);
   dpiMsg.set_bytesclientdelta(567);
   dpiMsg.set_bytesserverdelta(234);
   dpiMsg.set_packetsdelta(33);
   dpiMsg.set_sessionidq_proto_ymsg(2345);
   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushlightuserdata(luaState, &mRuleEngine);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SendInterFlowToSyslog(luaState));

   dpiMsg.set_timeupdate(567);
   dpiMsg.set_bytesserver(23456);
   dpiMsg.set_bytesclient(7890);
   dpiMsg.set_packettotal(124);
   dpiMsg.set_timedelta(111); // 567 - 456
   dpiMsg.set_bytesclientdelta(1101); // 7890 - 6789
   dpiMsg.set_bytesserverdelta(11111); // 23456 - 12345
   dpiMsg.set_packetsdelta(25); // 124 - 99

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_INTERMEDIATE_FINAL);
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushlightuserdata(luaState, &mRuleEngine);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SendInterFlowToSyslog(luaState));

   //std::cout << "SyslogOutput: " << sysLogOutput << std::endl;
   // Did the data show up in the syslog output
   ASSERT_EQ(2, sysLogOutput.size());
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("EVT:003 "));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find(testUuid));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,1007,567/6789,234/12345,33/99,123,456,222/333"));
   EXPECT_NE(std::string::npos, sysLogOutput[1].find("EVT:003 "));
   EXPECT_NE(std::string::npos, sysLogOutput[1].find(testUuid));
   EXPECT_NE(std::string::npos, sysLogOutput[1].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,1007,1101/7890,11111/23456,25/124,123,567,111/444"));
   lua_close(luaState);
#endif
}

TEST_F(LuaFunctionsTest, StaticCallSendFinalFlow) {
#ifdef LR_DEBUG
   MockRuleEngine mRuleEngine(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, 0);
   mRuleEngine.mSiemMode = true;
   mRuleEngine.mSiemDebugMode = false;

   DpiMsgLR dpiMsg;

   dpiMsg.set_flowtype(DpiMsgLRproto_Type_FINAL);

   string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
   dpiMsg.set_sessionid(testUuid.c_str());

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
   dpiMsg.set_ipsource(ipSrc);

   string testIpDst = "10.128.64.251";
   uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
   dpiMsg.set_ipdest(ipDst);

   string path("base.eth.ip.udp.ntp");
   dpiMsg.set_pktpath(path.c_str());

   string testIpSourcePort = "=12345"; // bogus, but easier to test
   dpiMsg.set_portsource(12345);

   string testIpDestPort = "=54321"; // bogus, but easier to test
   dpiMsg.set_portdest(54321);
   dpiMsg.set_protoid(12);
   dpiMsg.set_application_id_endq_proto_base(13);
   dpiMsg.add_application_endq_proto_base("wrong");
   dpiMsg.add_application_endq_proto_base("unknown");
   dpiMsg.set_bytesserver(12345);
   dpiMsg.set_bytesserverdelta(12345);
   dpiMsg.set_bytesclient(6789);
   dpiMsg.set_bytesclientdelta(6789);
   dpiMsg.set_packettotal(99);
   dpiMsg.set_packetsdelta(99);
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
   dpiMsg.set_timestart(123);
   dpiMsg.set_timeupdate(456);
   dpiMsg.set_timedelta(333);
   dpiMsg.set_sessionidq_proto_ymsg(2345);

   lua_State *luaState;
   luaState = luaL_newstate();
   lua_pushlightuserdata(luaState, &dpiMsg);
   lua_pushlightuserdata(luaState, &mRuleEngine);
   ASSERT_EQ(0, LuaRuleEngineFunctions::SendFinalFlowToSyslog(luaState));

   //std::cout << "SyslogOutput: " << sysLogOutput << std::endl;
   // Did the data show up in the syslog output
   ASSERT_EQ(1, sysLogOutput.size());
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("EVT:001 "));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find(testUuid));
   EXPECT_NE(std::string::npos, sysLogOutput[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,1007,6789/6789,12345/12345,99/99,123,456,333/333"));
   lua_close(luaState);
#endif
}

TEST_F(LuaFunctionsTest, StaticCallGetThreadId) {
#ifdef LR_DEBUG
   unsigned int expectedThreadId(5);
   MockRuleEngine mre(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, expectedThreadId);
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect the threadId to be 5 as initialized.
   lua_pushlightuserdata(luaState, &mre);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetThreadId(luaState));
   EXPECT_EQ(expectedThreadId, lua_tointeger(luaState, -1));
   lua_close(luaState);
#endif
}

TEST_F(LuaFunctionsTest, StaticCallGetRawMsgSize) {
#ifdef LR_DEBUG
   MockRuleEngine mre(conf, syslogName, syslogOption,
           syslogFacility, syslogPriority, true, 0);
   lua_State *luaState;
   luaState = luaL_newstate();

   // Expect the raw message size to match the initialized value of 0
   lua_pushlightuserdata(luaState, &mre);
   ASSERT_EQ(1, LuaRuleEngineFunctions::GetRawMsgSize(luaState));
   EXPECT_EQ(0, lua_tointeger(luaState, -1));
   lua_close(luaState);
#endif
}
