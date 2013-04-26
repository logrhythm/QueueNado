#include "LuaExecuterTest.h"
#include "LuaExecuter.h"
#include "luajit-2.0/lua.hpp"
static int LuaTestStringFunction(lua_State *L) {
   std::string stringArg = lua_tostring(L, 1);
   stringArg += " RanIt";
   lua_pushstring(L, stringArg.c_str());
   return 1;
}
TEST_F(LuaExecuterTest, RunIntRule) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return x/y end");
   rule.set_rulereturn(::protoMsg::RuleConf_Return_INTEGER);
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
TEST_F(LuaExecuterTest, BadRule) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test2 (x,y) return x/y end");
   rule.set_rulereturn(::protoMsg::RuleConf_Return_INTEGER);
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
TEST_F(LuaExecuterTest, RunRuleWithFunction) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test2");
   rule.set_repetitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test2 (x) return CFunction1(x) end");
   rule.set_rulereturn(::protoMsg::RuleConf_Return_INTEGER);
   rule.set_numberofreturnvals(1);
   rule.add_inputstrings("Test Program:");

   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
	executer.RegisterFunction("CFunction1",LuaTestStringFunction);
		  
   EXPECT_TRUE(executer.RunRule(rule, returnInts, returnBools, returnStrings));
   EXPECT_EQ(0, returnInts.size());
   EXPECT_EQ(0, returnBools.size());
   ASSERT_EQ(1, returnStrings.size());
   EXPECT_EQ("Test Program: RanIt", returnStrings[0]);
}
