#include "LuaExecuterTest.h"
#include "MockLuaExecuter.h"
#include "luajit-2.0/lua.hpp"

static int LuaTestStringFunction(lua_State *L) {
   std::string stringArg = lua_tostring(L, 1);
   stringArg += " RanIt";
   lua_pushstring(L, stringArg.c_str());
   return 1;
}

static int LuaTestDpiMsgFunction(lua_State *L) {
   networkMonitor::DpiMsgLR* dpiMsg = static_cast<networkMonitor::DpiMsgLR*> (lua_touserdata(L, 1));
   dpiMsg->set_sessionid("TEST");
   return 0;
}

static int LuaTestPacketFunction(lua_State *L) {
   networkMonitor::DpiMsgLR* dpiMsg = static_cast<networkMonitor::DpiMsgLR*> (lua_touserdata(L, 1));
   struct upacket* packet = static_cast<struct upacket*> (lua_touserdata(L, 2));
   dpiMsg->set_sessionid("TEST");
   packet->len = 999;
   return 0;
}

static int LuaTestPacketFunction2(lua_State *L) {
   networkMonitor::DpiMsgLR* dpiMsg = static_cast<networkMonitor::DpiMsgLR*> (lua_touserdata(L, 1));
   struct upacket* packet = static_cast<struct upacket*> (lua_touserdata(L, 2));
   dpiMsg->set_sessionid("TEST");
   packet->len = 99;
   return 0;
}

TEST_F(LuaExecuterTest, RunIntRule) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return x/y end");
   rule.set_numberofreturnvals(1);
   rule.add_inputintegers(8);
   rule.add_inputintegers(4);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   EXPECT_TRUE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   ASSERT_EQ(1, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
   EXPECT_EQ(2, returnInts[0]);
}

TEST_F(LuaExecuterTest, RunIntRuleWithoutArgs) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return x/y end");
   rule.set_numberofreturnvals(1);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   EXPECT_FALSE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   ASSERT_EQ(0, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
}

TEST_F(LuaExecuterTest, BadRule) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test2 (x,y) return x/y end");
   rule.set_numberofreturnvals(1);
   rule.add_inputintegers(8);
   rule.add_inputintegers(4);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   EXPECT_FALSE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   EXPECT_EQ(0, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
}

TEST_F(LuaExecuterTest, BadRule2) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return x/y");
   rule.set_numberofreturnvals(1);
   rule.add_inputintegers(8);
   rule.add_inputintegers(4);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   EXPECT_FALSE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   EXPECT_EQ(0, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
   EXPECT_FALSE(executer.RegisterRule(rule));
   EXPECT_EQ(0, executer.SizeOfRuleset(protoMsg::RuleConf_Type_GENERIC));
   EXPECT_TRUE(executer.RemoveRule(rule.ruletype(), rule.name()));
}

TEST_F(LuaExecuterTest, RunRuleWithFunction) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test2");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test2 (x) return CFunction1(x) end");
   rule.add_inputstrings("Test Program:");
   rule.set_numberofreturnvals(1);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   executer.RegisterFunction("CFunction1", LuaTestStringFunction);

   EXPECT_TRUE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   EXPECT_EQ(0, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   ASSERT_EQ(1, returnStrings.size());
   EXPECT_EQ("Test Program: RanIt", returnStrings[0]);
}

TEST_F(LuaExecuterTest, RunDpiMsgRule) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test3");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_FLOWCOMPLETE);
   rule.set_runforever(false);
   rule.set_ruletext("function test3 (x) return CFunction1(x) end");
   rule.set_numberofreturnvals(0);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   executer.RegisterFunction("CFunction1", LuaTestDpiMsgFunction);

   networkMonitor::DpiMsgLR dpiMsg;
   EXPECT_TRUE(executer.RunFlowRule(rule, &dpiMsg));
   EXPECT_EQ("TEST", dpiMsg.sessionid());
}

TEST_F(LuaExecuterTest, RunDpiMsgRuleBadFunction) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test3");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_FLOWCOMPLETE);
   rule.set_runforever(false);
   rule.set_ruletext("function test3 (x) return CFunction1(x) end");
   rule.set_numberofreturnvals(0);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   executer.RegisterFunction("CFunction1", LuaTestStringFunction);

   networkMonitor::DpiMsgLR dpiMsg;
   EXPECT_FALSE(executer.RunFlowRule(rule, &dpiMsg));
}


TEST_F(LuaExecuterTest, NonRunDpiMsgRule) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test3");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test3 (x) return CFunction1(x) end");
   rule.set_numberofreturnvals(0);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   executer.RegisterFunction("CFunction1", LuaTestDpiMsgFunction);

   networkMonitor::DpiMsgLR dpiMsg;
   EXPECT_FALSE(executer.RunFlowRule(rule, &dpiMsg));

}

TEST_F(LuaExecuterTest, RegisterPacketRule) {
   LuaExecuter executer;

   protoMsg::RuleConf rule;
   rule.set_name("test4");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test4 (x,y) return CFunction1(x,y) end");
   rule.set_numberofreturnvals(0);
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);
   EXPECT_EQ(0, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));
   EXPECT_TRUE(executer.RegisterRule(rule));
   EXPECT_EQ(1, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));

   networkMonitor::DpiMsgLR dpiMsg;
   struct upacket* packet = reinterpret_cast<struct upacket*> (malloc(sizeof (struct upacket)));
   packet->len = 0;
   std::vector<void*> args;
   args.push_back(&dpiMsg);
   args.push_back(packet);
   EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET, args));
   EXPECT_EQ("TEST", dpiMsg.sessionid());
   EXPECT_EQ(999, packet->len);
   free(packet);
}

TEST_F(LuaExecuterTest, GetLuaStateOnlyCompilesOnce) {
#ifdef LR_DEBUG
   MockLuaExecuter executer;

   protoMsg::RuleConf rule;
   rule.set_name("test4");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test4 (x,y) return CFunction1(x,y) end");
   rule.set_numberofreturnvals(0);
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);

   lua_State* luaState = executer.GetLuaState(rule);
   lua_State* luaState2 = executer.GetLuaState(rule);

   EXPECT_EQ(luaState, luaState2);
#endif
}

TEST_F(LuaExecuterTest, LoadStaticFunctionsOnlyLoadsOncePerRule) {
#ifdef LR_DEBUG
   MockLuaExecuter executer;

   protoMsg::RuleConf rule;
   rule.set_name("test4");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test4 (x,y) return CFunction1(x,y) end");
   rule.set_numberofreturnvals(0);
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);

   lua_State* luaState = executer.GetLuaState(rule);
   EXPECT_EQ(1, executer.GetRegisteredFunctions(luaState).size());
   executer.LoadStaticCFunctions(luaState);
   EXPECT_EQ(1, executer.GetRegisteredFunctions(luaState).size());
#endif
}

TEST_F(LuaExecuterTest, NoStaticFunctions) {
#ifdef LR_DEBUG
   MockLuaExecuter executer;

   protoMsg::RuleConf rule;
   rule.set_name("test4");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test4 (x,y) return true end");
   rule.set_numberofreturnvals(0);

   lua_State* luaState = executer.GetLuaState(rule);
   EXPECT_EQ(0, executer.GetRegisteredFunctions(luaState).size());
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);
   EXPECT_EQ(1, executer.GetRegisteredFunctions(luaState).size());
#endif
}

TEST_F(LuaExecuterTest, ChangeInputVariables) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return x/y end");
   rule.set_numberofreturnvals(1);
   rule.add_inputintegers(8);
   rule.add_inputintegers(4);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   EXPECT_TRUE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   ASSERT_EQ(1, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
   EXPECT_EQ(2, returnInts[0]);

   rule.set_inputintegers(0, 9);
   rule.set_inputintegers(1, 3);
   EXPECT_TRUE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   ASSERT_EQ(1, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
   EXPECT_EQ(3, returnInts[0]);
}

TEST_F(LuaExecuterTest, ChangeScript) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_GENERIC);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return x/y end");
   rule.set_numberofreturnvals(1);
   rule.add_inputintegers(8);
   rule.add_inputintegers(4);

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   EXPECT_TRUE(executer.RegisterRule(rule));
   EXPECT_TRUE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   ASSERT_EQ(1, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
   EXPECT_EQ(2, returnInts[0]);
   executer.RemoveRule(rule.ruletype(), rule.name());
   rule.set_ruletext("function test1 (x,y) return x*y end");

   EXPECT_TRUE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   ASSERT_EQ(1, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   EXPECT_EQ(0, returnStrings.size());
   EXPECT_EQ(32, returnInts[0]);
}

TEST_F(LuaExecuterTest, ReRegisterPacketRule) {
   LuaExecuter executer;

   protoMsg::RuleConf rule;
   rule.set_name("test4");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test4 (x,y) return CFunction1(x,y) end");
   rule.set_numberofreturnvals(0);
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);
   executer.RegisterFunction("CFunction2", LuaTestPacketFunction2);
   EXPECT_EQ(0, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));
   EXPECT_TRUE(executer.RegisterRule(rule));
   EXPECT_EQ(1, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));

   networkMonitor::DpiMsgLR dpiMsg;
   struct upacket* packet = reinterpret_cast<struct upacket*> (malloc(sizeof (struct upacket)));
   packet->len = 0;
   std::vector<void*> args;
   args.push_back(&dpiMsg);
   args.push_back(packet);
   EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET, args));
   EXPECT_EQ("TEST", dpiMsg.sessionid());
   EXPECT_EQ(999, packet->len);
   packet->len = 1;
   rule.set_ruletext("function test4 (x,y) return CFunction2(x,y) end");
   EXPECT_TRUE(executer.RegisterRule(rule));
   EXPECT_EQ(1, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));
   EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET, args));
   EXPECT_EQ("TEST", dpiMsg.sessionid());
   EXPECT_EQ(99, packet->len);

   free(packet);
}

TEST_F(LuaExecuterTest, PacketRuleBenchmarkReBuilds) {
#ifndef LR_DEBUG

   LuaExecuter executer;

   protoMsg::RuleConf rule;
   rule.set_name("test4");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test4 (x,y) return CFunction1(x,y) end");
   rule.set_numberofreturnvals(0);
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);
   EXPECT_EQ(0, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));
   unsigned int iterations(100000);
   networkMonitor::DpiMsgLR dpiMsg;
   struct upacket* packet = reinterpret_cast<struct upacket*> (malloc(sizeof (struct upacket)));
   packet->len = 0;
   std::vector<void*> args;
   args.push_back(&dpiMsg);
   args.push_back(packet);
   StartTimedSection(.00018, iterations);
   for (unsigned int i = 0; i < iterations; i++) {
      EXPECT_TRUE(executer.RegisterRule(rule));
      EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET, args));
      EXPECT_TRUE(executer.RemoveRule(rule.ruletype(), rule.name()));
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   free(packet);
#endif

}

TEST_F(LuaExecuterTest, PacketRuleBenchmark) {
#ifndef LR_DEBUG
   LuaExecuter executer;

   protoMsg::RuleConf rule;
   rule.set_name("test4");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test4 (x,y) return CFunction1(x,y) end");
   rule.set_numberofreturnvals(0);
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);
   EXPECT_EQ(0, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));
   unsigned int iterations(100000);
   networkMonitor::DpiMsgLR dpiMsg;
   struct upacket* packet = reinterpret_cast<struct upacket*> (malloc(sizeof (struct upacket)));
   packet->len = 0;
   std::vector<void*> args;
   args.push_back(&dpiMsg);
   args.push_back(packet);
   EXPECT_TRUE(executer.RegisterRule(rule));
   StartTimedSection(.0000010, iterations);
   for (unsigned int i = 0; i < iterations; i++) {

      EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET, args));

   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   free(packet);

#endif
}

TEST_F(LuaExecuterTest, ReadScriptsFromDirectories) {
#ifdef LR_DEBUG
   MockLuaExecuter executer;

   executer.ClearScriptLocation();
   executer.SetScriptLocation(protoMsg::RuleConf_Type_PACKET, "resources/luatestpacket");
   
   ASSERT_TRUE(executer.Initialize());

   ASSERT_EQ(1,executer.NumberOfFunctionFiles(protoMsg::RuleConf_Type_PACKET));
   ASSERT_EQ("test1",executer.GetFunctionFileName(protoMsg::RuleConf_Type_PACKET,0));
   ASSERT_EQ("function test1 (x,y) return CFunction1(x,y) end\n", 
           executer.GetFunctionFileContents(protoMsg::RuleConf_Type_PACKET,"test1"));
   
   executer.RegisterFunction("CFunction1", LuaTestPacketFunction);
   executer.RegisterFunction("CFunction2", LuaTestPacketFunction2);
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return CFunction2(x,y) end");
   rule.set_numberofreturnvals(0);
   EXPECT_TRUE(executer.RegisterRule(rule));
   EXPECT_EQ(1, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));
   
   networkMonitor::DpiMsgLR dpiMsg;
   struct upacket* packet = reinterpret_cast<struct upacket*> (malloc(sizeof (struct upacket)));
   packet->len = 0;
   std::vector<void*> args;
   args.push_back(&dpiMsg);
   args.push_back(packet);
   EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET, args));
   EXPECT_EQ("TEST", dpiMsg.sessionid());
   EXPECT_EQ(999, packet->len); // Indicating CFunction1 ran (ignoring the rule text but using the file instead)
   packet->len = 0;
   rule.set_ruletext("");
   EXPECT_TRUE(executer.RegisterRule(rule));
   EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET, args));
   EXPECT_EQ(999, packet->len); // no need for rule text if the rule is already defined in a file
   free(packet);
#endif
}