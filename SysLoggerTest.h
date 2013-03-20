#pragma once

#include "gtest/gtest.h"
#include "SysLogger.h"
#include <string.h>
#include <cstdarg>

// Mock syslog data and routines to use in testing
static std::string sysLogOpenIdent;
static int sysLogOpenOption;
static int sysLogOpenFacility;
static int sysLogPriority;
static std::string sysLogOutput;
static bool bLogOpen;

const int MaxSyslogSize=2048;

void openlog(const char *ident, int option, int facility);
void syslog(int priority, const char *format, ...);
void closelog(void);


class SysLoggerTests : public ::testing::Test
{
public:
    SysLoggerTests(){};

protected:
	virtual void SetUp() {
      sysLogOpenIdent.clear();
      sysLogOpenOption = 0;
      sysLogOpenFacility = 0;
      sysLogPriority = 0;
      sysLogOutput.clear();
      bLogOpen = false;
   }
	virtual void TearDown() {
      EXPECT_FALSE( bLogOpen );
      EXPECT_EQ( sysLogOpenIdent, sysLogOutput );
   }

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
   sysLogPriority = priority;
   sysLogOutput.clear();
   va_start( arguments, format );
   vsnprintf( output, MaxSyslogSize, format, arguments );
   va_end( arguments );
   sysLogOutput = output;
}

void closelog(void) {
   // Clean up the test data
   sysLogOpenIdent.clear();
   sysLogOpenOption = 0;
   sysLogOpenFacility = 0;
   sysLogPriority = 0;
   sysLogOutput.clear();
   bLogOpen = false;
}

