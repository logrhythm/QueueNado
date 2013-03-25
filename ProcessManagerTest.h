#pragma once

#include "gtest/gtest.h"
#include "ProcessManager.h"
#include "czmq.h"

class ProcessManagerTest : public ::testing::Test {
public:

    ProcessManagerTest() {
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