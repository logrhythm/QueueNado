#include <stdlib.h>
#include "LuaFunctionsTest.h"
#include "MockConf.h"
#include "g2logworker.hpp"
#include "g2log.hpp"
#include "MockLuaExecuter.h"
#include "LuaPacketFunctions.h"
static int LuaTestFunction(lua_State *L) {
   return 0;
}

static int LuaTestFunction2(lua_State *L) {
   return 0;
}
TEST_F(LuaFunctionsTest, AddRemoveFromContainer) {
   MockLuaFunctions functions;
   
   ASSERT_EQ(0,functions.NumberOfFunctions());
   functions.AddFunction("Test1",LuaTestFunction);
   ASSERT_EQ(1,functions.NumberOfFunctions());
   functions.AddFunction("Test1",LuaTestFunction2);
   ASSERT_EQ(1,functions.NumberOfFunctions());
   functions.RemoveFunction("Test1");
   ASSERT_EQ(0,functions.NumberOfFunctions());
}

TEST_F(LuaFunctionsTest, AddThenRegister) {
   MockLuaFunctions functions;
   
   ASSERT_EQ(0,functions.NumberOfFunctions());
   functions.AddFunction("Test1",LuaTestFunction);
   ASSERT_EQ(1,functions.NumberOfFunctions());
   functions.AddFunction("Test2",LuaTestFunction2);
   ASSERT_EQ(2,functions.NumberOfFunctions());

   MockLuaExecuter ruleEngine;
   functions.RegisterAllKnownFunctions(ruleEngine);
   
   std::map<std::string, lua_CFunction>  registered = ruleEngine.GetPossibleFunctions();
   
   ASSERT_TRUE(registered["Test1"] == LuaTestFunction);
   ASSERT_TRUE(registered["Test2"] == LuaTestFunction2);
}

TEST_F(LuaFunctionsTest, PacketFunctions) {
   LuaPacketFunctions functions;
   MockConf conf;
   conf.mPCapCaptureLocation="/tmp";
   functions.Initialize(conf);
   MockLuaExecuter ruleEngine;
   functions.RegisterAllKnownFunctions(ruleEngine);
   
   std::map<std::string, lua_CFunction>  registered = ruleEngine.GetPossibleFunctions();
   ASSERT_TRUE(registered.end()!=registered.find("SessionAge"));
   ASSERT_TRUE(registered.end()!=registered.find("EndOfFlow"));
   ASSERT_TRUE(registered.end()!=registered.find("GetCurrentClassification"));
   ASSERT_TRUE(registered.end()!=registered.find("CapturePacket"));
   ASSERT_TRUE(registered.end()!=registered.find("WriteAllCapturedPackets"));
   ASSERT_TRUE(registered.end()!=registered.find("CleanupCapturedPackets"));
}