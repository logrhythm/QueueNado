#pragma once

#include <fstream>
#include "gtest/gtest.h"
#include "SyslogConfig.h"
#include <string.h>
#include <stdio.h>

using namespace std;

// Mock system call data and prototype
static string configSyslogSystemCall;
int system(const char *command);


class SyslogConfigTests : public ::testing::Test
{
public:
   SyslogConfigTests(){};

protected:

   string mTestModScriptName;

	virtual void SetUp() {
   }
   virtual void TearDown() {
      // Check if the file was deleted
      ofstream testModSyslogFile;
      testModSyslogFile.open( mTestModScriptName.c_str(), ios::in );

      EXPECT_FALSE( testModSyslogFile.is_open() );
   }

};

int system(const char *command)
{
   configSyslogSystemCall.clear();
   configSyslogSystemCall = command;
   return 0;
}
