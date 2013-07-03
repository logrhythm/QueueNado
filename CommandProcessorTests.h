#pragma once

#include "gtest/gtest.h"
#include "CommandProcessor.h"
#include <czmq.h>
#include <boost/thread.hpp>
class CommandProcessorTests : public ::testing::Test {
public:

    CommandProcessorTests() {
       zctx_interrupted = false;
       srand(std::time(NULL));
    };
protected:
    
    virtual void SetUp() {
        
    };

    virtual void TearDown() {

    };
private:

};

