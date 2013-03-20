/* 
 * File:   ConfProcessorTests.h
 * Author: Ben Aldrich
 *
 * Created on September 28, 2012, 3:53 PM
 */

#pragma once

#include "gtest/gtest.h"
#include "ConfProcessor.h"
#include "boost/lexical_cast.hpp"
#include <csignal>

class ConfProcessorTests : public ::testing::Test {
public:

    ConfProcessorTests() {
    };
protected:
    std::string mWriteLocation;
    std::string mTestConf;

    virtual void SetUp() {
        int pid = getpid();
        mWriteLocation = "/tmp/test.yaml.";
        mWriteLocation += boost::lexical_cast<std::string > (pid);
        remove(mWriteLocation.c_str());
        mTestConf = "resources/test.yaml";

    };

    virtual void TearDown() {
       std::string syslogWriteLocation = mWriteLocation;
       syslogWriteLocation += ".Syslog";
       std::string qosmosWriteLocation = mWriteLocation;
       qosmosWriteLocation == ".Qosmos";
       remove(mWriteLocation.c_str());
       remove(syslogWriteLocation.c_str());
       remove(qosmosWriteLocation.c_str());
    };
private:

};

