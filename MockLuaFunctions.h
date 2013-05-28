#pragma once
#include "LuaFunctions.h"

class MockLuaFunctions : public LuaFunctions {
public:

   MockLuaFunctions() : LuaFunctions() {
   }

   virtual ~MockLuaFunctions() {
   }

   void AddFunction(const std::string& functionName, lua_CFunction function) {
      LuaFunctions::AddFunction(functionName,function);
   }

   void RemoveFunction(const std::string& functionName) {
      LuaFunctions::RemoveFunction(functionName);
   }
   size_t NumberOfFunctions() {
      return mAvaliableFunctions.size();
   }
};

