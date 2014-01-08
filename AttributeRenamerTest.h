#pragma once

#include "gtest/gtest.h"
#include "AttributeRenamer.h"

class AttributeRenamerTest : public ::testing::Test
{
public:
    AttributeRenamerTest(){

    };

protected:
	virtual void SetUp() {}
	virtual void TearDown() {}
 
   std::string mAddress;
};

