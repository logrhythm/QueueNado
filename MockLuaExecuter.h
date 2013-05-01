#pragma once
#include "LuaExecuter.h"

class MockLuaExecuter : public LuaExecuter {
public:

   MockLuaExecuter() {
   }

   ~MockLuaExecuter() {
   }
   void LoadStaticCFunctions(lua_State* luaState) {
      return LuaExecuter::LoadStaticCFunctions(luaState);
   }
   int LoadVariables(const protoMsg::RuleConf& rule, lua_State* luaState) {
      return LuaExecuter::LoadVariables(rule,luaState);
   }
   bool ParseReturnStack(lua_State* luaState, std::vector<bool>& returnBools,
           std::vector<std::string>& returnStrings, std::vector<int>& returnInts) {
      return LuaExecuter::ParseReturnStack(luaState,returnBools,returnStrings,returnInts);
   }
   lua_State* GetLuaState(const protoMsg::RuleConf& rule) {
      return LuaExecuter::GetLuaState(rule);
   }
};

