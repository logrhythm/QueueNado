check_PROGRAMS=nmTest \
CommandProcessorTests \
MsgUuidTest \
SysLoggerTest \
ZMQTest \
PacketCaptureTest \
DpiMsgLRTest \
QosmosCodeGeneratorTest \
QosmosDpiTest \
ConfProcessorTests \
ProcessManagerTest \
RuleEngineTest \
StatsAccumulatorTest \
SendStatsTest \
ShotgunAlienTests \
BoomStickTest \
RifleVampireTests \
TestTCPIPDirectionlessHash \
LuaExecuterTest \
ProcStatsTest \
JSONEncoderTest \
transferZeromqTest \
RESTBuilderTest \
ElasticSearchTest \
ConversionTest \
DiskPacketCaptureTest \
DiskCleanupTest \
DiskUsageTest \
ThreadContainerTest \
ToolsTest \
AttributeRenamerTest \
ForkerPipeTest \
SyslogReporterTest

BASE_LIBS=-Wl,-z -Wl,now \
          -Wl,-R${abs_top_srcdir}/ \
          -Wl,-R${abs_top_srcdir}/lib \
          -Wl,-R${abs_top_srcdir}/source/libZMQ/ \
          -Wl,-R${abs_top_srcdir}/source/libfork/ \
          -Wl,-R${abs_top_srcdir}/source/liblrdpi/ \
          -Wl,-R${abs_top_srcdir}/source/libsyslog/ \
          -Wl,-R${abs_top_srcdir}/source/libstats/ \
          -Wl,-R${abs_top_srcdir}/source/libconf/ \
          -Wl,-R${abs_top_srcdir}/source/libcapture/ \
          -Wl,-R${abs_top_srcdir}/source/libcommand/ \
          -Wl,-R${abs_top_srcdir}/source/libprocessclient/ \
          -Wl,-R${abs_top_srcdir}/source/libprocess/ \
          -Wl,-R${abs_top_srcdir}/source/libelasticsearch/ \
          -Wl,-R${abs_top_srcdir}/source/libtools/ \
          -Wl,-R${abs_top_srcdir}/source/libglobal/ \
          -Wl,-R${abs_top_srcdir}/source/libsystem/ \
          -Wl,-R${abs_top_srcdir}/source/libremap/ \
          -Wl,-R/usr/local/probe/lib/ \
          -Wl,-R/usr/local/probe/lib64/ \
          -Wl,-R${prefix}/lib \
          -L${abs_top_srcdir}/source/libZMQ/ \
          -L${abs_top_srcdir}/source/libfork/ \
          -L${abs_top_srcdir}/source/liblrdpi/ \
          -L${abs_top_srcdir}/source/libsyslog/ \
          -L${abs_top_srcdir}/source/libstats/ \
          -L${abs_top_srcdir}/source/libconf/ \
          -L${abs_top_srcdir}/source/libcapture/ \
          -L${abs_top_srcdir}/source/libcommand/ \
          -L${abs_top_srcdir}/source/libprocessclient/ \
          -L${abs_top_srcdir}/source/libprocess/ \
          -L${abs_top_srcdir}/source/liblua/ \
          -L${abs_top_srcdir}/source/libelasticsearch/ \
          -L${abs_top_srcdir}/source/libtools/ \
          -L${abs_top_srcdir}/source/libglobal/ \
          -L${abs_top_srcdir}/source/libsystem/ \
          -L${abs_top_srcdir}/source/libremap/ \
          -L${abs_top_srcdir}/source/ \
          -L/usr/local/probe/lib/ \
          -L/usr/local/probe/lib64/ \
          -L${prefix}/lib \
          -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free 

INCLUDES=-I${abs_top_srcdir}/source/ \
         -I${abs_top_srcdir}/source/libZMQ \
         -I${abs_top_srcdir}/source/liblrdpi \
         -I${abs_top_srcdir}/source/libfork \
         -I${abs_top_srcdir}/source/libsyslog \
         -I${abs_top_srcdir}/source/libstats \
         -I${abs_top_srcdir}/source/libconf \
         -I${abs_top_srcdir}/source/libcapture \
         -I${abs_top_srcdir}/source/libcommand \
         -I${abs_top_srcdir}/source/libprocessclient \
         -I${abs_top_srcdir}/source/libprocess \
         -I${abs_top_srcdir}/source/liblua \
         -I${abs_top_srcdir}/source/libelasticsearch \
         -I${abs_top_srcdir}/source/libtools \
         -I${abs_top_srcdir}/source/libglobal \
         -I${abs_top_srcdir}/source/libsystem \
         -I${abs_top_srcdir}/source/libremap \
         -I/usr/local/probe/include \
         -include /usr/include/fenv.h

DYNAMIC_LIBS=-ltcmalloc \
             -llib_g2log \
             -lsyslog -lZMQ -llrdpi -lstats -lprocessclient -lprocess -lgmock -lgtest -lpthread -ldl -lstdc++ -lpcap -lzmq -lczmq -lcommand \
             -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity  \
             -lqosmosauto -lboost_regex -lboost_thread -lboost_system -lcapture -lyaml-cpp -lconf -lconfmaster \
             -lluajit-5.1 -llua -ljansson -lelasticsearch -ltools -lsystem -ltcmalloc_minimal -lfork -lforkmother -lremap

ForkerPipeTest_LDADD=${DYNAMIC_LIBS} -lfork
ForkerPipeTest_LDFLAGS=${BASE_LIBS}
ForkerPipeTest_SOURCES=main.cpp \
	ForkerPipeTest.cpp

ZMQTest_LDADD=${DYNAMIC_LIBS}
ZMQTest_LDFLAGS=${BASE_LIBS}
ZMQTest_SOURCES=main.cpp \
	ZeroMQTests.cpp \
	TestSendReceivePacketZMQ.cpp \
	CZMQToolkitTests.cpp \
	CrowbarHeadcrabTests.cpp \
	TestSendReceiveDpiMsgLRZMQ.cpp

ShotgunAlienTests_LDADD=${DYNAMIC_LIBS}
ShotgunAlienTests_LDFLAGS=${BASE_LIBS}
ShotgunAlienTests_SOURCES=main.cpp ShotgunAlienTests.cpp
ShotgunAlienTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

BoomStickTest_LDADD=${DYNAMIC_LIBS}
BoomStickTest_LDFLAGS=${BASE_LIBS}
BoomStickTest_SOURCES=main.cpp BoomStickTest.cpp
BoomStickTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

RifleVampireTests_LDADD=${DYNAMIC_LIBS}
RifleVampireTests_LDFLAGS=${BASE_LIBS}
RifleVampireTests_SOURCES=main.cpp RifleVampireTests.cpp
RifleVampireTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

QosmosCodeGeneratorTest_LDADD=${DYNAMIC_LIBS}
QosmosCodeGeneratorTest_LDFLAGS=${BASE_LIBS}
QosmosCodeGeneratorTest_SOURCES=main.cpp QosmosCodeGeneratorTests.cpp

QosmosDpiTest_LDADD=${DYNAMIC_LIBS}
QosmosDpiTest_LDFLAGS=${BASE_LIBS}
QosmosDpiTest_SOURCES=main.cpp \
QosmosDpiTest.cpp \
QosmosPacketAllocatorTests.cpp \
QosmosProtocolCaptureTest.cpp

DiskPacketCaptureTest_LDADD=${DYNAMIC_LIBS}
DiskPacketCaptureTest_LDFLAGS=${BASE_LIBS}
DiskPacketCaptureTest_SOURCES=main.cpp \
DiskPacketCaptureTest.cpp DiskPcapMoveTest.cpp

DiskCleanupTest_LDADD=${DYNAMIC_LIBS}
DiskCleanupTest_LDFLAGS=${BASE_LIBS}
DiskCleanupTest_SOURCES=main.cpp \
DiskCleanupTest.cpp

PacketCaptureTest_LDADD=${DYNAMIC_LIBS}
PacketCaptureTest_LDFLAGS=${BASE_LIBS}
PacketCaptureTest_SOURCES=main.cpp PacketCaptureTest.cpp


DiskUsageTest_LDADD=${DYNAMIC_LIBS}
DiskUsageTest_LDFLAGS=${BASE_LIBS}
DiskUsageTest_SOURCES=main.cpp DiskUsageTest.cpp MockPcapDiskUsage.h


MsgUuidTest_LDADD=${DYNAMIC_LIBS}
MsgUuidTest_LDFLAGS=${BASE_LIBS}
MsgUuidTest_SOURCES=main.cpp MsgUuidTest.cpp

ConfProcessorTests_LDADD=${DYNAMIC_LIBS}
ConfProcessorTests_LDFLAGS=${BASE_LIBS}
ConfProcessorTests_SOURCES=main.cpp \
        ConfProcessorTests.cpp \
	QosmosConfTest.cpp \
	ConfNtpProcessorTests.cpp \
	ConfBaseValidationTests.cpp \
	ConfSyslogValidationTests.cpp 
ConfProcessorTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

CommandProcessorTests_LDADD=${DYNAMIC_LIBS}
CommandProcessorTests_LDFLAGS=${BASE_LIBS}
CommandProcessorTests_SOURCES=main.cpp \
        CommandProcessorTests.cpp \
        CommandFactoryTests.cpp \
        UpgradeCommandTest.h  \
        ShutdownCommandTest.cpp  \
        DriveInfoCommandTest.cpp  \
        PCapFilePathCommandTest.cpp  \
        NetworkConfigCommandTest.h  \
        TestProtoDefaultsMethods.cpp \
        MockProtoDefaults.h \
        NtpConfigCommandTest.cpp \
        NtpConfigCommandTest.h \
        MockNtpConfigCommand.h \
        MockRestartSyslogCommand.h \
        MockConfNtpToReadFile.h \
        MockConfNtp.h \
        MockProcessManagerCommand.h \
	MockDriveInfo.h \
	ConfigRequestCommandTest.cpp \
	ConfigRequestCommandTest.h \
	MockConfigRequestCommand.h
CommandProcessorTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\" ${INCLUDES}  -DGTEST_LINKED_AS_SHARED_LIBRARY=1


SysLoggerTest_LDADD=${DYNAMIC_LIBS}
SysLoggerTest_LDFLAGS=${BASE_LIBS}
SysLoggerTest_SOURCES=main.cpp SysLoggerTest.cpp
SysLoggerTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1


RuleEngineTest_LDADD=${DYNAMIC_LIBS}
RuleEngineTest_LDFLAGS=${BASE_LIBS}
RuleEngineTest_SOURCES=main.cpp RuleEngineTest.cpp
RuleEngineTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1


StatsAccumulatorTest_LDADD=${DYNAMIC_LIBS}
StatsAccumulatorTest_LDFLAGS=${BASE_LIBS}
StatsAccumulatorTest_SOURCES=main.cpp StatsAccumulatorTest.cpp
StatsAccumulatorTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1


SendStatsTest_LDADD=${DYNAMIC_LIBS}
SendStatsTest_LDFLAGS=${BASE_LIBS}
SendStatsTest_SOURCES=main.cpp SendStatsTest.cpp
SendStatsTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1


ProcStatsTest_LDADD=${DYNAMIC_LIBS}
ProcStatsTest_LDFLAGS=${BASE_LIBS}
ProcStatsTest_SOURCES=main.cpp ProcStatsTest.cpp TCMallocStatsTest.cpp ProcMemStatsTest.cpp
ProcStatsTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

ProcessManagerTest_LDADD=${DYNAMIC_LIBS}
ProcessManagerTest_LDFLAGS=${BASE_LIBS}
ProcessManagerTest_SOURCES=main.cpp ProcessManagerTest.cpp
ProcessManagerTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1


DpiMsgLRTest_LDADD=${DYNAMIC_LIBS}
DpiMsgLRTest_LDFLAGS=${BASE_LIBS}
DpiMsgLRTest_SOURCES=main.cpp DpiMsgLRTest.cpp DpiMsgLRPoolTest.cpp
DpiMsgLRTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1


TestTCPIPDirectionlessHash_LDADD=${DYNAMIC_LIBS}
TestTCPIPDirectionlessHash_LDFLAGS=${BASE_LIBS}
TestTCPIPDirectionlessHash_SOURCES=main.cpp \
        TestTCPIPDirectionlessHash.cpp
TestTCPIPDirectionlessHash_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

LuaExecuterTest_LDADD=${DYNAMIC_LIBS}
LuaExecuterTest_LDFLAGS=${BASE_LIBS}
LuaExecuterTest_SOURCES=main.cpp \
        LuaExecuterTest.cpp \
        LuaFunctionsTest.cpp
LuaExecuterTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  \
-DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

ThreadContainerTest_LDADD=${DYNAMIC_LIBS}
ThreadContainerTest_LDFLAGS=${BASE_LIBS}
ThreadContainerTest_SOURCES=main.cpp ThreadContainerTest.cpp
ThreadContainerTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

ToolsTest_LDADD=${DYNAMIC_LIBS}
ToolsTest_LDFLAGS=${BASE_LIBS}

ToolsTest_SOURCES=main.cpp \
ToolsTestMakeUnique.cpp \
ToolsTestStringFix.cpp \
ToolsTestStopWatch.cpp \
ToolsTestFileIO.cpp \
ToolsTestUuidHash.cpp \
Death.cpp Death.h \
ToolsTestConcurrent.cpp \
EthInfoTest.cpp \
WorkerThreadTest.cpp \
ToolsRangeTest.cpp
ToolsTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

JSONEncoderTest_LDADD=${DYNAMIC_LIBS}
JSONEncoderTest_LDFLAGS=${BASE_LIBS}
JSONEncoderTest_SOURCES=main.cpp JSONEncoderTest.cpp
JSONEncoderTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

transferZeromqTest_LDADD=${DYNAMIC_LIBS}
transferZeromqTest_LDFLAGS=${BASE_LIBS}
transferZeromqTest_SOURCES=main.cpp transferZeromqTest.cpp
transferZeromqTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

RESTBuilderTest_LDADD=${DYNAMIC_LIBS}
RESTBuilderTest_LDFLAGS=${BASE_LIBS}
RESTBuilderTest_SOURCES=main.cpp RESTBuilderTest.cpp RESTParserTest.cpp
RESTBuilderTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

ElasticSearchTest_LDADD=${DYNAMIC_LIBS}
ElasticSearchTest_LDFLAGS=${BASE_LIBS}
ElasticSearchTest_SOURCES=main.cpp ElasticSearchTest.cpp
ElasticSearchTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

ConversionTest_LDADD=${DYNAMIC_LIBS}
ConversionTest_LDFLAGS=${BASE_LIBS}
ConversionTest_SOURCES=main.cpp ConversionTest.cpp
ConversionTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

nmTest_LDADD=${DYNAMIC_LIBS}
nmTest_LDFLAGS=${BASE_LIBS}
nmTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1
nmTest_SOURCES=main.cpp \
 CommandFactoryTests.cpp \
 CommandProcessorTests.cpp \
 DpiMsgLRTest.cpp \
 DpiMsgLRPoolTest.cpp \
 CrowbarHeadcrabTests.cpp \
 CZMQToolkitTests.cpp \
 MsgUuidTest.cpp \
 PacketCaptureTest.cpp \
 ProcessManagerTest.cpp \
 ProcStatsTest.cpp \
 TCMallocStatsTest.cpp \
 ProcMemStatsTest.cpp \
 QosmosDpiTest.cpp \
 QosmosCodeGeneratorTests.cpp \
 QosmosPacketAllocatorTests.cpp \
 QosmosProtocolCaptureTest.cpp \
 QosmosConfTest.cpp \
 RifleVampireTests.cpp \
 RuleEngineTest.cpp \
 SendStatsTest.cpp \
 ShotgunAlienTests.cpp \
 BoomStickTest.cpp \
 StatsAccumulatorTest.cpp \
 TestTCPIPDirectionlessHash.cpp \
 TestSendReceiveDpiMsgLRZMQ.cpp \
 TestSendReceivePacketZMQ.cpp \
 LuaExecuterTest.cpp \
 DiskPacketCaptureTest.cpp \
 ZeroMQTests.cpp \
 JSONEncoderTest.cpp \
 RESTBuilderTest.cpp \
 ElasticSearchTest.cpp \
 ThreadContainerTest.cpp \
 DriveInfoCommandTest.cpp \
 PCapFilePathCommandTest.cpp \
 ToolsTestMakeUnique.cpp \
 ToolsTestStringFix.cpp \
 ToolsTestStopWatch.cpp \
 TestProtoDefaultsMethods.cpp
#	SysLoggerTest.cpp \  conflicts with Rule Engine

SyslogReporterTest_LDADD=${DYNAMIC_LIBS}
SyslogReporterTest_LDFLAGS=${BASE_LIBS}
SyslogReporterTest_SOURCES=main.cpp SyslogReporterTest.cpp MockSyslogReporter.h Death.cpp Death.h
SyslogReporterTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1

AttributeRenamerTest_LDADD=${DYNAMIC_LIBS}
AttributeRenamerTest_LDFLAGS=${BASE_LIBS}
AttributeRenamerTest_SOURCES=main.cpp AttributeRenamerTest.cpp MockAttributeRenamer.h
AttributeRenamerTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBUILD_VERSION=\"@PROBE_BUILD_VERSION@\"  -DGTEST_LINKED_AS_SHARED_LIBRARY=1
