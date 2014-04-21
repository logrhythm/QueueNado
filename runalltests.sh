#!/bin/sh
export LANG=C
chmod 777 .libs
chmod 777 .libs/* 
chmod 777 .
chmod 777 ..
chmod 777 ../source/.libs
chmod 777 ../source/.libs/*
touch ../ZMQTest.junit.xml ../PacketCaptureTest.junit.xml ../DpiMsgLRTest.junit.xml ../QosmosCodeGeneratorTest.junit.xml ../QosmosDpiTest.junit.xml ../SysLoggerTest.junit.xml ../ConfProcessorTest.junit.xml ../SendStatsTest.junit.xml ../StatsAccumulatorTest.junit.xml ../ShotgunAlienTests.junit.xml ../RifleVampireTests.junit.xml ../TestTCPIPDirectionlessHash.junit.xml ../RuleEngineTest.junit.xml ../CommandProcessorTests.junit.xml ../ProcessManagerTest.junit.xml ../LuaExecuterTest.junit.xml ../ThreadContainerTest.junit.xml ../../RESTBuilderTest.junit.xml ../JSONEncoderTest.junit.xml ../ConversionTest.junit.xml ../DiskPacketCaptureTest.junit.xml ../DiskCleanupTest.junit.xml ../ElasticSearchTest.xml ../BoomStickTest.xml ../SyslogReporterTest.xml ../DiskUsageTest.xml ../AttributeRenamerTest.junit.xml ../ForkerPipeTest.junit.xml ../DeathTest.junit.xml
chmod 777 ../*.xml
export LUA_PATH=/usr/local/probe/bin/\?.lua\;/usr/local/probe/share/lua/5.1/\?.lua\;/usr/local/probe/share/lua/5.1/\?/init.lua\;/usr/local/probe/libLua/\?.lua\;/usr/local/probe/apiLua/\?.lua\;/usr/local/probe/apiLua/usr/\?.lua\;\;
export LUA_CPATH=/usr/local/probe/lib/lua/5.1/\?.so\;/usr/local/probe/libLua/\?.lua\;/usr/local/probe/apiLua/\?.lua\;/usr/local/probe/apiLua/usr/\?.lua\;
./ZMQTest --gtest_output=xml:../ZMQTest.junit.xml
./BoomStickTest --gtest_output=xml:../BoomStickTest.junit.xml
./ShotgunAlienTests --gtest_output=xml:../ShotgunAlienTests.junit.xml
./RifleVampireTests --gtest_output=xml:../RifleVampireTests.junit.xml
./PacketCaptureTest --gtest_output=xml:../PacketCaptureTest.junit.xml
./DpiMsgLRTest --gtest_output=xml:../DpiMsgLRTest.junit.xml
./QosmosCodeGeneratorTest --gtest_output=xml:../QosmosCodeGeneratorTest.junit.xml
./QosmosDpiTest --gtest_output=xml:../QosmosDpiTest.junit.xml
./SysLoggerTest --gtest_output=xml:../SysLoggerTest.junit.xml
./SendStatsTest --gtest_output=xml:../SendStatsTest.junit.xml
./StatsAccumulatorTest --gtest_output=xml:../StatsAccumulatorTest.junit.xml
./TestTCPIPDirectionlessHash --gtest_output=xml:../TestTCPIPDirectionlessHash.junit.xml
./RESTBuilderTest --gtest_output=xml:../RESTBuilderTest.junit.xml
./ThreadContainerTest --gtest_output=xml:../ThreadContainerTest.junit.xml
../source/MotherForker ./LuaExecuterTest --gtest_output=xml:../LuaExecuterTest.junit.xml
./JSONEncoderTest --gtest_output=xml:../JSONEncoderTest.junit.xml
./ConversionTest --gtest_output=xml:../ConversionTest.junit.xml
./DiskPacketCaptureTest --gtest_output=xml:../DiskPacketCaptureTest.junit.xml
./ElasticSearchTest --gtest_output=xml:../ElasticSearchTest.junit.xml
./AttributeRenamerTest --gtest_output=xml:../AttributeRenamerTest.junit.xml
./ForkerPipeTest --gtest_output=xml:../ForkerPipeTest.junit.xml
./DeathTest --gtest_output=xml:../DeathTest.junit.xml
../source/MotherForker ./RuleEngineTest --gtest_output=xml:../RuleEngineTest.junit.xml
../source/MotherForker ./SyslogReporterTest --gtest_output=xml:../SyslogReporterTest.junit.xml
../source/MotherForker ./DiskCleanupTest --gtest_output=xml:../DiskCleanupTest.junit.xml
../source/MotherForker ./DiskUsageTest --gtest_output=xml:../DiskUsageTest.junit.xml
../source/MotherForker ./CommandProcessorTests --gtest_output=xml:../CommandProcessorTests.junit.xml
../source/MotherForker ./ProcessManagerTest --gtest_output=xml:../ProcessManagerTest.junit.xml
../source/MotherForker ./ConfProcessorTests --gtest_output=xml:../ConfProcessorTest.junit.xml


