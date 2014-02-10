/* 
 * File:   SyslogReporterTest.h
 * Author: John Gress
 *
 */

#pragma once

#include "gtest/gtest.h"
#include "MockSyslogReporter.h"
#include "ConfSlave.h"
#include "ConfMaster.h"
#include <cstdlib> // rand
#include <ctime>
#include <string>
#include <vector>
#include "ProcessManager.h"
static int syslogOption = LOG_ODELAY | LOG_PERROR | LOG_PID;
static int syslogFacility = LOG_LOCAL4;
static int syslogPriority = LOG_DEBUG;
static std::string syslogName("LogRhythmDpiTest");

// Mock syslog data and routines to use in testing
static std::string syslogOpenIdent;
static int syslogOpenOption;
static int syslogOpenFacility;
static int syslogOpenPriority;
static std::vector<std::string> syslogOutput;
static bool bLogOpen;
const int MaxSyslogSize = 2048;

void openlog(const char *ident, int option, int facility);
void syslog(int priority, const char *format, ...);
void closelog(void);


class SyslogReporterTest : public ::testing::Test {
public:

   SyslogReporterTest() : mConfSlave(ConfSlave::Instance()),
                          mMasterConf(ConfMaster::Instance()) {
      mMasterConf.SetPath("resources/test.yaml");
      mMasterConf.Start();

      mConfSlave.Start();
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      std::srand(std::time(0)); // current time as seed for random generator
   }

   ~SyslogReporterTest() {
      mConfSlave.Stop();
      mMasterConf.Stop();
   }

   void ShootZeroCopySyslogThread(int numberOfMessages, std::string& location, 
         std::string& exampleData);
protected:
   ConfSlave& mConfSlave;
   ConfMaster& mMasterConf;

   virtual void SetUp() {
      ProcessManager::Instance();
      //std::cout << "setup" << std::endl;
      syslogOpenIdent.clear();
      syslogOpenOption = 0;
      syslogOpenFacility = 0;
      syslogOpenPriority = 0;
      syslogOutput.clear();
      bLogOpen = false;
      //std::cout << "setup-end" << std::endl;
   };

   virtual void TearDown() {
      EXPECT_FALSE(bLogOpen);
   };

private:

};

void openlog(const char *ident, int option, int facility) {
   syslogOpenIdent.clear();
   syslogOpenIdent = ident;
   syslogOpenOption = option;
   syslogOpenFacility = facility;
   bLogOpen = true;
}

void syslog(int priority, const char *format, ...) {
   char output[MaxSyslogSize];
   va_list arguments;
   syslogOpenPriority = priority;
   va_start(arguments, format);
   vsnprintf(output, MaxSyslogSize, format, arguments);
   va_end(arguments);
   syslogOutput.push_back(output);
}

void closelog(void) {
   // Clean up the test data
   syslogOpenIdent.clear();
   syslogOpenOption = 0;
   syslogOpenFacility = 0;
   syslogOpenPriority = 0;
   syslogOutput.clear();
   bLogOpen = false;
}

