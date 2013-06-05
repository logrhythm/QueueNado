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

static int syslogOption = LOG_ODELAY | LOG_PERROR | LOG_PID;
static int syslogFacility = LOG_LOCAL4;
static int syslogPriority = LOG_DEBUG;
static string syslogName("LogRhythmDpiTest");


// Mock syslog data and routines to use in testing
static string sysLogOpenIdent;
static int sysLogOpenOption;
static int sysLogOpenFacility;
static int sysLogOpenPriority;
static std::vector<string> sysLogOutput;
static bool bLogOpen;
const int MaxSyslogSize = 2048;

void openlog(const char *ident, int option, int facility);
void syslog(int priority, const char *format, ...);
void closelog(void);

class RuleEngineTest : public ::testing::Test {
public:

   RuleEngineTest() : conf(networkMonitor::ConfSlave::Instance()), masterConf(networkMonitor::ConfMaster::Instance()) {
      //std::cout << "Pre" << std::endl;
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
      //std::cout << "setup" << std::endl;
      sysLogOpenIdent.clear();
      sysLogOpenOption = 0;
      sysLogOpenFacility = 0;
      sysLogOpenPriority = 0;
      sysLogOutput.clear();
      bLogOpen = false;
      //std::cout << "setup-end" << std::endl;
   };

   virtual void TearDown() {
      EXPECT_FALSE(bLogOpen);
      //EXPECT_EQ( sysLogOpenIdent, sysLogOutput );
   };

   networkMonitor::ConfSlave& conf;
   networkMonitor::ConfMaster& masterConf;
#ifdef LR_DEBUG
   networkMonitor::MockDpiMsgLR tDpiMessage;
#else
   networkMonitor::DpiMsgLR tDpiMessage;
#endif
private:

};

void openlog(const char *ident, int option, int facility) {
   sysLogOpenIdent.clear();
   sysLogOpenIdent = ident;
   sysLogOpenOption = option;
   sysLogOpenFacility = facility;
   bLogOpen = true;
}

void syslog(int priority, const char *format, ...) {
   char output[MaxSyslogSize];
   va_list arguments;
   sysLogOpenPriority = priority;
   va_start(arguments, format);
   vsnprintf(output, MaxSyslogSize, format, arguments);
   va_end(arguments);
   sysLogOutput.push_back(output);
}

void closelog(void) {
   // Clean up the test data
   sysLogOpenIdent.clear();
   sysLogOpenOption = 0;
   sysLogOpenFacility = 0;
   sysLogOpenPriority = 0;
   sysLogOutput.clear();
   bLogOpen = false;
}

