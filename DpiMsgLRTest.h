#pragma once

#include "gtest/gtest.h"
#include "DpiMsgLR.h"
#include "MockDpiMsgLR.h"



class DpiMsgLRTests : public ::testing::Test
{
public:
    DpiMsgLRTests(){};

protected:
	virtual void SetUp() {}
	virtual void TearDown() {}
	networkMonitor::MockDpiMsgLR tDpiMessage;
};
