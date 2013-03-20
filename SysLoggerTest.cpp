#include <stdlib.h>
#include "SysLoggerTest.h"
#include <string>

using namespace networkMonitor;
using namespace std;


TEST_F( SysLoggerTests, sysLoggerConstructor1 )
{
   // Test the ability to start the SysLogger using the no-argument constructor.
   SysLogger syslog;

   EXPECT_EQ( syslog.getLogName(), sysLogOpenIdent );
   EXPECT_EQ( syslog.getOpenOption(), sysLogOpenOption );
   EXPECT_EQ( syslog.getOpenFacility(), sysLogOpenFacility );
   EXPECT_TRUE( bLogOpen );

   // Log something
   string msg("Network Monitor SysLogger Test");
   syslog.sendLog( msg );

   EXPECT_EQ( msg, sysLogOutput );
   EXPECT_EQ( syslog.getLogPriority(), sysLogPriority );

   SysLogger* pSyslogger = new SysLogger;
   delete pSyslogger;
}

TEST_F( SysLoggerTests, sysLoggerConstructor2 )
{
   // Test the ability to start the SysLogger using the constructor with a
   // syslog name.
   std::string logName( "NMTestLog" );
   int option = LOG_ODELAY | LOG_PERROR | LOG_PID;
   int facility = LOG_LOCAL2;
   int priority = LOG_WARNING;

   SysLogger syslog( logName, option, facility, priority);
   EXPECT_EQ( logName, sysLogOpenIdent );
   EXPECT_EQ( option, sysLogOpenOption );
   EXPECT_EQ( facility, sysLogOpenFacility );
   EXPECT_TRUE( bLogOpen );

   // Log something
   string msg("Network Monitor NMTestLog");
   syslog.sendLog( msg );
   EXPECT_EQ( msg, sysLogOutput );
   EXPECT_EQ( priority, sysLogPriority );
}

