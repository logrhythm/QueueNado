#pragma once

#include "gtest/gtest.h"
#include "LuaFunctions.h"
#include "MockLuaFunctions.h"

class LuaFunctionsTest : public ::testing::Test
{
public:
    LuaFunctionsTest(){};

protected:
	virtual void SetUp() {}
	virtual void TearDown() {}
};

