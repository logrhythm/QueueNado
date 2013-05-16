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
   std::set<std::string> GetRegisteredFunctions(lua_State* luaState) {
      return LuaExecuter::GetRegisteredFunctions(luaState);
   }
   void ClearScriptLocation() {
      mRuleLocations.clear();
   }
   void SetScriptLocation(protoMsg::RuleConf_Type type, boost::filesystem::path path) {
      mRuleLocations[type] = path;
   }
   size_t NumberOfFunctionFiles(protoMsg::RuleConf_Type type) {
      return mLoadedRules[type].size();
   }
   std::string GetFunctionFileName(protoMsg::RuleConf_Type type,size_t number) {
      size_t count(0);
      std::map<std::string, std::string>::iterator it = mLoadedRules[type].begin();
      for ( ; it != mLoadedRules[type].end() && count < number ; it++ ) {
         count++;
      }
      return (it)->first;
   }
   std::string GetFunctionFileContents(protoMsg::RuleConf_Type type,std::string name) {
      return mLoadedRules[type][name];
   }

};

