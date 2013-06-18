check_PROGRAMS=nmTest \
CommandProcessorTests \
MsgUuidTest \
SyslogConfigTest \
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
RifleVampireTests \
TestTCPIPDirectionlessHash \
LuaExecuterTest \
ProcStatsTest \
JSONEncoderTest \
ThreadContainerTest

BASE_LIBS=-Wl,-z -Wl,now \
          -Wl,-R${abs_top_srcdir}/ \
          -Wl,-R${abs_top_srcdir}/lib \
          -Wl,-R${abs_top_srcdir}/source/libZMQ/ \
          -Wl,-R${abs_top_srcdir}/source/liblrdpi/ \
          -Wl,-R${abs_top_srcdir}/source/libstats/ \
          -Wl,-R${abs_top_srcdir}/source/libconf/ \
          -Wl,-R${abs_top_srcdir}/source/libcapture/ \
          -Wl,-R${abs_top_srcdir}/source/libcommand/ \
          -Wl,-R${abs_top_srcdir}/source/libprocess/ \
          -Wl,-R/usr/local/probe/lib/ \
          -Wl,-R/usr/local/probe/lib64/ \
          -Wl,-R${prefix}/lib \
          -L${abs_top_srcdir}/source/libZMQ/ \
          -L${abs_top_srcdir}/source/liblrdpi/ \
          -L${abs_top_srcdir}/source/libstats/ \
          -L${abs_top_srcdir}/source/libconf/ \
          -L${abs_top_srcdir}/source/libcapture/ \
          -L${abs_top_srcdir}/source/libcommand/ \
          -L${abs_top_srcdir}/source/libprocess/ \
          -L${abs_top_srcdir}/source/liblua/ \
          -L${abs_top_srcdir}/source/ \
          -L/usr/local/probe/lib/ \
          -L/usr/local/probe/lib64/ \
          -L${prefix}/lib

INCLUDES=-I${abs_top_srcdir}/source/ \
         -I${abs_top_srcdir}/source/libZMQ \
         -I${abs_top_srcdir}/source/liblrdpi \
         -I${abs_top_srcdir}/source/libstats \
         -I${abs_top_srcdir}/source/libconf \
         -I${abs_top_srcdir}/source/libcapture \
         -I${abs_top_srcdir}/source/libcommand \
         -I${abs_top_srcdir}/source/libprocess \
         -I${abs_top_srcdir}/source/liblua \
         -I/usr/local/probe/include

ZMQTest_LDADD=-lnm -lZMQ -llua -llrdpi -lpthread -ldl -lstdc++ -lgtest -lpcap \
              -lzmq -lczmq -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata \
              -lqmsecurity -lboost_system -lboost_thread -lstats -lyaml-cpp -llib_g2log -ljansson
ZMQTest_LDFLAGS=${BASE_LIBS}
ZMQTest_SOURCES=main.cpp \
	ZeroMQTests.cpp \
	TestSendReceivePacketZMQ.cpp \
	CZMQToolkitTests.cpp \
	CrowbarHeadcrabTests.cpp \
	TestSendReceiveDpiMsgLRZMQ.cpp

ShotgunAlienTests_LDADD=-lnm -lZMQ -lczmq -llua -llrdpi -lpthread -ldl -lstdc++ -lgtest -lpcap \
              -lzmq -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata \
              -lqmsecurity -lboost_system -lboost_thread -lstats -lyaml-cpp -llib_g2log -ljansson
ShotgunAlienTests_LDFLAGS=${BASE_LIBS}
ShotgunAlienTests_SOURCES=main.cpp ShotgunAlienTests.cpp
ShotgunAlienTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"

RifleVampireTests_LDADD=-lnm -lZMQ -lczmq -llua -llrdpi -lpthread -ldl -lstdc++ -lgtest -lpcap \
              -lzmq -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata \
              -lqmsecurity -lboost_system -lboost_thread -lstats -lyaml-cpp -llib_g2log -ljansson
RifleVampireTests_LDFLAGS=${BASE_LIBS}
RifleVampireTests_SOURCES=main.cpp RifleVampireTests.cpp
RifleVampireTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


QosmosCodeGeneratorTest_LDADD=-lpthread -ldl -lstdc++ -lgtest -lz -lboost_regex\
	-lqosmosauto -lnm -lZMQ -lboost_thread -lZMQ -lstats \
	-lstdc++ -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity \
	-lboost_system -lboost_thread -lcapture -llua -llrdpi -llib_g2log -ljansson
QosmosCodeGeneratorTest_LDFLAGS=${BASE_LIBS}
QosmosCodeGeneratorTest_SOURCES=main.cpp QosmosCodeGeneratorTests.cpp

QosmosDpiTest_LDADD=-lnm -lZMQ -llua -llrdpi -lstats -lpthread -ldl -lstdc++ -lgtest -lpcap -lzmq -lczmq \
             -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity \
             -lqosmosauto -lboost_regex -lboost_thread -lboost_system -lyaml-cpp -lcapture -lconf -llib_g2log -ljansson
QosmosDpiTest_LDFLAGS=${BASE_LIBS}
QosmosDpiTest_SOURCES=main.cpp QosmosDpiTest.cpp QosmosPacketAllocatorTests.cpp QosmosProtocolCaptureTest.cpp DiskPacketCaptureTest.cpp 



PacketCaptureTest_LDADD=-lnm -lZMQ -llua -llrdpi -lstats -lpthread -ldl -lstdc++ -lgtest \
                        -lpcap -lzmq -lczmq -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl \
                        -lqmdata -lqmsecurity -lboost_system -lyaml-cpp \
			-lboost_thread -lboost_system -lcapture -lconf -llib_g2log -ljansson
PacketCaptureTest_LDFLAGS=${BASE_LIBS}
PacketCaptureTest_SOURCES=main.cpp PacketCaptureTest.cpp

MsgUuidTest_LDADD=-lnm -lZMQ -llua -llrdpi -lstats -lstdc++ -lgtest -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system -lyaml-cpp -llib_g2log -ljansson
MsgUuidTest_LDFLAGS=${BASE_LIBS}
MsgUuidTest_SOURCES=main.cpp MsgUuidTest.cpp

SyslogConfigTest_LDADD=-lnm -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols -lqmctl -lconf -llua -llrdpi\
		       -lqmdata -lqmsecurity -lyaml-cpp -lcapture -llib_g2log -lstats -lZMQ -lboost_thread -lboost_system  -ljansson
SyslogConfigTest_LDFLAGS=${BASE_LIBS}
SyslogConfigTest_SOURCES=main.cpp SyslogConfigTest.cpp

ConfProcessorTests_LDADD=-lnm -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols \
 -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system -lprotobuf \
 -lstats -lyaml-cpp -lconf -llua -llrdpi -llib_g2log -ljansson
ConfProcessorTests_LDFLAGS=${BASE_LIBS}
ConfProcessorTests_SOURCES=main.cpp \
        ConfProcessorTests.cpp \
	QosmosConfTest.cpp
ConfProcessorTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"

CommandProcessorTests_LDADD=-lnm -lZMQ -lstdc++ -lgtest -lprotobuf -lstats \
 -lconf -llua -llrdpi -lprocess -llib_g2log -lcommand -lboost_system -ljansson
CommandProcessorTests_LDFLAGS=${BASE_LIBS}
CommandProcessorTests_SOURCES=main.cpp \
        CommandProcessorTests.cpp \
        CommandFactoryTests.cpp \
        UpgradeCommandTest.h  \
        NetworkConfigCommandTest.h  \
	MockProcessManagerCommand.h
CommandProcessorTests_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\" ${INCLUDES}

SysLoggerTest_LDADD=-lnm -lstdc++ -lgtest -lqmengine -lqmprotocols -lqmctl -lqmdata \
	            -lqmsecurity -lZMQ -lstats -lyaml-cpp -lboost_thread -lboost_system -llua -llrdpi -llib_g2log -ljansson
SysLoggerTest_LDFLAGS=${BASE_LIBS}
SysLoggerTest_SOURCES=main.cpp SysLoggerTest.cpp
SysLoggerTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


RuleEngineTest_LDADD=-lnm -llua -llrdpi -lstats -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols -lconf \
                    -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system \
                    -lprotobuf -lyaml-cpp -llib_g2log -ljansson
RuleEngineTest_LDFLAGS=${BASE_LIBS}
RuleEngineTest_SOURCES=main.cpp RuleEngineTest.cpp
RuleEngineTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


StatsAccumulatorTest_LDADD=-lnm -llua -llrdpi -lstats -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols -lconf \
                    -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system \
                    -lprotobuf -lyaml-cpp -llib_g2log -ljansson
StatsAccumulatorTest_LDFLAGS=${BASE_LIBS}
StatsAccumulatorTest_SOURCES=main.cpp StatsAccumulatorTest.cpp
StatsAccumulatorTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


SendStatsTest_LDADD=-lnm -llua -llrdpi -lstats -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system -lprotobuf -lyaml-cpp -llib_g2log -ljansson
SendStatsTest_LDFLAGS=${BASE_LIBS}
SendStatsTest_SOURCES=main.cpp SendStatsTest.cpp
SendStatsTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


ProcStatsTest_LDADD=-lnm -llua -llrdpi -lstats -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system -lprotobuf -lyaml-cpp -llib_g2log -ljansson
ProcStatsTest_LDFLAGS=${BASE_LIBS}
ProcStatsTest_SOURCES=main.cpp ProcStatsTest.cpp
ProcStatsTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"

ProcessManagerTest_LDADD=-lnm -llua -llrdpi -lstats -lZMQ -lstdc++ -lgtest -lboost_system -lprotobuf -llib_g2log -lzmq -lczmq -lprocess -ljansson
ProcessManagerTest_LDFLAGS=${BASE_LIBS}
ProcessManagerTest_SOURCES=main.cpp ProcessManagerTest.cpp
ProcessManagerTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


DpiMsgLRTest_LDADD=-lnm -lZMQ -llua -llrdpi -lstats -lpthread -ldl -lstdc++ -lgtest -lpcap -lzmq -lczmq -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system -lyaml-cpp -llib_g2log  -ljansson
DpiMsgLRTest_LDFLAGS=${BASE_LIBS}
DpiMsgLRTest_SOURCES=main.cpp DpiMsgLRTest.cpp DpiMsgLRPoolTest.cpp
DpiMsgLRTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


TestTCPIPDirectionlessHash_LDADD=-lnm -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system -lprotobuf -lstats -lyaml-cpp -lconf -llrdpi -llib_g2log -lcapture -ljansson
TestTCPIPDirectionlessHash_LDFLAGS=${BASE_LIBS}
TestTCPIPDirectionlessHash_SOURCES=main.cpp \
        TestTCPIPDirectionlessHash.cpp
TestTCPIPDirectionlessHash_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"

LuaExecuterTest_LDADD=-lstdc++ -lgtest \
 -lboost_thread -lboost_system -lprotobuf \
 -llib_g2log -lluajit-5.1 -llua -lnm -lZMQ -llrdpi -lstats \
 -lzmq -lczmq   -ljansson
LuaExecuterTest_LDFLAGS=${BASE_LIBS}
LuaExecuterTest_SOURCES=main.cpp \
        LuaExecuterTest.cpp \
        LuaFunctionsTest.cpp
LuaExecuterTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  \
-DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"

ThreadContainerTest_LDADD=-lnm -llua -llrdpi -lstats -lZMQ -lstdc++ -lgtest -lqmengine -lqmprotocols -lconf \
                    -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system \
                    -lprotobuf -lyaml-cpp -llib_g2log -ljansson
ThreadContainerTest_LDFLAGS=${BASE_LIBS}
ThreadContainerTest_SOURCES=main.cpp ThreadContainerTest.cpp
ThreadContainerTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"

JSONEncoderTest_LDADD=-lnm -lZMQ -llua -llrdpi -lstats -lpthread -ldl -lstdc++ -lgtest -lpcap -lzmq -lczmq -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity -lboost_thread -lboost_system -lyaml-cpp -llib_g2log  -ljansson
JSONEncoderTest_LDFLAGS=${BASE_LIBS}
JSONEncoderTest_SOURCES=main.cpp JSONEncoderTest.cpp
JSONEncoderTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"


nmTest_LDADD=-lnm -lZMQ -llrdpi -lstats -lprocess -lpthread -ldl -lstdc++ -lgtest -lpcap -lzmq -lczmq -lcommand \
             -lz -lprotobuf -lqmengine -lqmprotocols -lqmctl -lqmdata -lqmsecurity  \
             -lqosmosauto -lboost_regex -lboost_thread -lboost_system -lcapture -lyaml-cpp -lconf -llib_g2log  -lluajit-5.1 -llua -ljansson

nmTest_LDFLAGS=${BASE_LIBS}
nmTest_CPPFLAGS =  -DINSTALL_PREFIX=\"$(prefix)\"  -DBULID_VERSION=\"@PROBE_BUILD_VERSION@\"
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
 QosmosDpiTest.cpp \
 QosmosCodeGeneratorTests.cpp \
 QosmosPacketAllocatorTests.cpp \
 QosmosProtocolCaptureTest.cpp \
 QosmosConfTest.cpp \
 RifleVampireTests.cpp \
 RuleEngineTest.cpp \
 SendStatsTest.cpp \
 ShotgunAlienTests.cpp \
 StatsAccumulatorTest.cpp \
 SyslogConfigTest.cpp \
 TestTCPIPDirectionlessHash.cpp \
 TestSendReceiveDpiMsgLRZMQ.cpp \
 TestSendReceivePacketZMQ.cpp \
 LuaExecuterTest.cpp \
 DiskPacketCaptureTest.cpp \
 ZeroMQTests.cpp \
 JSONEncoderTest.cpp \
 ThreadContainerTest.cpp

#	SysLoggerTest.cpp \  conflicts with Rule Engine
