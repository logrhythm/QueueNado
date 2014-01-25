/* 
 * File:   RuleEngineTest.h
 * Author: Robert Weber
 *
 */

#pragma once

#include "gtest/gtest.h"
#include "RuleEngine.h"
#include "MockRuleEngine.h"
#include "MockDpiMsgLR.h"
#include <string>
#include <cstdarg>
#include "ConfSlave.h"
#include "ConfMaster.h"
#include <memory>
#include "boost/lexical_cast.hpp"


class RuleEngineTest : public ::testing::Test {
public:

   RuleEngineTest() : conf(ConfSlave::Instance()), masterConf(networkMonitor::
      masterConf.SetPath("resources/test.yaml");
      masterConf.Start();

      conf.Start();
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      //std::cout << "Pre-end" << std::endl;
   };

   ~RuleEngineTest() {
      conf.Stop();
      masterConf.Stop();
   }

   std::string BuildExpectedHeaderForSiem(const std::string& expectedHeader,
           const std::string& expectedHeader2, unsigned int index) {
      std::string expected = expectedHeader;
      if (index < 10) {
         expected += "0";
      }
      if (index < 100) {
         expected += boost::lexical_cast<std::string>(index);
      } else {
         expected += "**";
      }
      expected += expectedHeader2;
      return std::move(expected);
   }
protected:

   virtual void SetUp() {
   };

   virtual void TearDown() {
   };

   ConfSlave& conf;
   ConfMaster& masterConf;
#ifdef LR_DEBUG
   networkMonitor::MockDpiMsgLR tDpiMessage;
#else
   networkMonitor::DpiMsgLR tDpiMessage;
#endif
private:

};

