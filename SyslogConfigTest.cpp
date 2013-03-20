#include <stdlib.h>
#include "SyslogConfigTest.h"
#include <string>

using namespace networkMonitor;
using namespace std;


TEST_F( SyslogConfigTests, SyslogConfigConstructor )
{
	// Instantiate a SyslogConfig object using the default data.
	SyslogConfig slc;

	// Check the data which was defaulted by the constructor.
	EXPECT_EQ( slc.getModSyslogScript(), SyslogConfig::DefaultModSyslogScript);
	slc.setScriptsDir("../scripts");

	// Save the name of the file to be used in a teardown test
	mTestModScriptName.clear();
	mTestModScriptName = SyslogConfig::DefaultModSyslogScript;

	// A system call was made to make the script executable
	string expectedCmd = SyslogConfig::ChmodSystemCall;
	expectedCmd += slc.getModSyslogScript();

	// The system command used should match the expectedCmd
	if (geteuid() == 0 ){
		EXPECT_EQ( configSyslogSystemCall, expectedCmd );

		// Check if the file exists in the expected location.
		ofstream testModSyslogFile;
		testModSyslogFile.open( slc.getModSyslogScript().c_str(), ios::in );

		EXPECT_TRUE( testModSyslogFile.is_open() );

		testModSyslogFile.close();
	}
	SyslogConfig* pConfig = new SyslogConfig;
	delete pConfig;
}
