#pragma once

#include "gtest/gtest.h"
#include "QosmosPacketAllocatorMock.h"

class QsomosPacketAllocatorTests : public ::testing::Test {
public:
	QsomosPacketAllocatorTests(){}
protected:
	virtual void SetUp() {

	}
	virtual void TearDown() {}
	MockQosmosPacketAllocator t_allocator;
};
