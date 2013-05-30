#pragma once

#include "gtest/gtest.h"
#include "LuaFunctions.h"
#include "MockLuaFunctions.h"
#include "ConfMaster.h"
#include "ConfSlave.h"
static int syslogOption = LOG_ODELAY | LOG_PERROR | LOG_PID;
static int syslogFacility = LOG_LOCAL4;
static int syslogPriority = LOG_DEBUG;
static std::string syslogName("LogRhythmDpiTest");
static std::string sysLogOpenIdent;
static int sysLogOpenOption;
static int sysLogOpenFacility;
static int sysLogOpenPriority;
static std::vector<std::string> sysLogOutput;
static bool bLogOpen;
const int MaxSyslogSize = 2048;

void openlog(const char *ident, int option, int facility);
void syslog(int priority, const char *format, ...);
void closelog(void);
class LuaFunctionsTest : public ::testing::Test {
public:

   LuaFunctionsTest() : conf(networkMonitor::ConfSlave::Instance()), masterConf(networkMonitor::ConfMaster::Instance()){

      masterConf.SetPath("resources/test.yaml");
      masterConf.Start();

      conf.Start();
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   };

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
   }

   virtual void TearDown() {
   }
   networkMonitor::ConfMaster& masterConf;
   networkMonitor::ConfSlave& conf;
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

