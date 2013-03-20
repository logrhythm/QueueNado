/* 
 * File:   ProcStatsTest.h
 *
 * Created on December 27, 2012, 12:21 PM
 */
#pragma once

#include "gtest/gtest.h"
#include "ProcStats.h"
class ProcStatsTest : public ::testing::Test
{
public:
    ProcStatsTest() : mProcStats(ProcStats::Instance()) {};

protected:
	virtual void SetUp() {}
	virtual void TearDown() {}
	ProcStats& mProcStats;
};
