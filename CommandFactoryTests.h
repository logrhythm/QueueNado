#pragma once

#include "gtest/gtest.h"
#include "CommandFactory.h"

class CommandFactoryTests : public ::testing::Test {
public:

    CommandFactoryTests() {
 //      zctx_interrupted = false;
       srand(std::time(NULL));
    };
protected:
    
    virtual void SetUp() {
        
    };

    virtual void TearDown() {

    };
private:

};

