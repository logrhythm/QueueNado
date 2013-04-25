#include "LuaExecuterTest.h"
#include "LuaExecuter.h"

TEST_F(LuaExecuterTest, RunIntRule) {
   LuaExecuter executer;
   protoMsg::RuleConf rule;
   rule.set_name("test1");
   rule.set_repitions(1);
   rule.set_ruletype(::protoMsg::RuleConf_Type_PACKET);
   rule.set_runforever(false);
   rule.set_ruletext("function test1 (x,y) return x+y end");
   rule.set_rulereturn(::protoMsg::RuleConf_Return_INTEGER);
   rule.set_numberofreturnvals(1);
   rule.add_inputintegers(4);
   rule.add_inputintegers(5);
   
   std::vector<int> returnInts;
   std::vector<bool> returnBools;
   std::vector<std::string> returnStrings;
   EXPECT_TRUE(executer.RunRule(rule,returnInts,returnBools,returnStrings));
   EXPECT_EQ(1,returnInts.size());
   EXPECT_EQ(0,returnBools.size());
   EXPECT_EQ(0,returnStrings.size());
   EXPECT_EQ(9,returnInts[0]);
}