#include "LuaExecuterTest.h"
#include "LuaExecuter.h"
#include "luajit-2.0/lua.hpp"

static int LuaTestStringFunction(lua_State *L) {
    std::string stringArg = lua_tostring(L, 1);
    stringArg += " RanIt";
    lua_pushstring(L, stringArg.c_str());
    return 1;
}

static int LuaTestDpiMsgFunction(lua_State *L) {
    networkMonitor::DpiMsgLR* dpiMsg = static_cast<networkMonitor::DpiMsgLR*> (lua_touserdata(L, 1));
    dpiMsg->set_uuid("TEST");
    return 0;
}

static int LuaTestPacketFunction(lua_State *L) {
    networkMonitor::DpiMsgLR* dpiMsg = static_cast<networkMonitor::DpiMsgLR*> (lua_touserdata(L, 1));
    ctb_ppacket packet = static_cast<ctb_ppacket> (lua_touserdata(L, 2));
    dpiMsg->set_uuid("TEST");
    packet->len = 999;
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
    EXPECT_EQ("TEST", dpiMsg.uuid());
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

TEST_F(LuaExecuterTest, NULLRunDpiMsgRule) {
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

    EXPECT_FALSE(executer.RunFlowRule(rule, NULL));

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

    EXPECT_EQ(0, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));
    executer.RegisterRule(rule);
    EXPECT_EQ(1, executer.SizeOfRuleset(protoMsg::RuleConf_Type_PACKET));

    executer.RegisterFunction("CFunction1", LuaTestPacketFunction);

    networkMonitor::DpiMsgLR dpiMsg;
    struct upacket* packet = reinterpret_cast<struct upacket*> (malloc(sizeof (struct upacket)));
    packet->len = 0;
    EXPECT_TRUE(executer.RunAllRules(protoMsg::RuleConf_Type_PACKET,&dpiMsg,packet));
    EXPECT_EQ("TEST", dpiMsg.uuid());
    EXPECT_EQ(999, packet->len);
    free(packet);
} 