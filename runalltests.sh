#!/bin/sh
export LANG=C
chmod 777 .libs
chmod 777 .libs/* 
chmod 777 .
chmod 777 ..
chmod 777 ../source/.libs
chmod 777 ../source/.libs/*
touch ../ZMQTest.junit.xml ../PacketCaptureTest.junit.xml ../DpiMsgLRTest.junit.xml ../QosmosCodeGeneratorTest.junit.xml ../QosmosDpiTest.junit.xml ../SysLoggerTest.junit.xml ../ConfProcessorTest.junit.xml ../SendStatsTest.junit.xml ../StatsAccumulatorTest.junit.xml ../ShotgunAlienTests.junit.xml ../RifleVampireTests.junit.xml ../TestTCPIPDirectionlessHash.junit.xml ../RuleEngineTest.junit.xml ../CommandProcessorTests.junit.xml ../ProcessManagerTest.junit.xml ../LuaExecuterTest.junit.xml ../ThreadContainerTest.junit.xml ../../RESTBuilderTest.junit.xml ../JSONEncoderTest.junit.xml ../ConversionTest.junit.xml ../DiskPacketCaptureTest.junit.xml ../DiskCleanupTest.junit.xml ../ElasticSearchTest.xml ../BoomStickTest.xml ../SyslogReporterTest.xml ../DiskUsageTest.xml
chmod 777 ../*.xml
export LUA_PATH=/usr/local/probe/bin/\?.lua\;/usr/local/probe/share/lua/5.1/\?.lua\;/usr/local/probe/share/lua/5.1/\?/init.lua\;/usr/local/probe/libLua/\?.lua\;/usr/local/probe/apiLua/\?.lua\;/usr/local/probe/apiLua/usr/\?.lua\;\;
export LUA_CPATH=/usr/local/probe/lib/lua/5.1/\?.so\;/usr/local/probe/libLua/\?.lua\;/usr/local/probe/apiLua/\?.lua\;/usr/local/probe/apiLua/usr/\?.lua\;
../source/MotherForker ./ZMQTest --gtest_output=xml:../ZMQTest.junit.xml
../source/MotherForker ./PacketCaptureTest --gtest_output=xml:../PacketCaptureTest.junit.xml
../source/MotherForker ./DpiMsgLRTest --gtest_output=xml:../DpiMsgLRTest.junit.xml
../source/MotherForker ./QosmosCodeGeneratorTest --gtest_output=xml:../QosmosCodeGeneratorTest.junit.xml
../source/MotherForker ./QosmosDpiTest --gtest_output=xml:../QosmosDpiTest.junit.xml
../source/MotherForker ./SysLoggerTest --gtest_output=xml:../SysLoggerTest.junit.xml
../source/MotherForker ./ConfProcessorTests --gtest_output=xml:../ConfProcessorTest.junit.xml
../source/MotherForker ./SendStatsTest --gtest_output=xml:../SendStatsTest.junit.xml
../source/MotherForker ./StatsAccumulatorTest --gtest_output=xml:../StatsAccumulatorTest.junit.xml
../source/MotherForker ./ShotgunAlienTests --gtest_output=xml:../ShotgunAlienTests.junit.xml
../source/MotherForker ./RifleVampireTests --gtest_output=xml:../RifleVampireTests.junit.xml
../source/MotherForker ./RuleEngineTest --gtest_output=xml:../RuleEngineTest.junit.xml
../source/MotherForker ./TestTCPIPDirectionlessHash --gtest_output=xml:../TestTCPIPDirectionlessHash.junit.xml
../source/MotherForker ./CommandProcessorTests --gtest_output=xml:../CommandProcessorTests.junit.xml
../source/MotherForker ./ProcessManagerTest --gtest_output=xml:../ProcessManagerTest.junit.xml
../source/MotherForker ./LuaExecuterTest --gtest_output=xml:../LuaExecuterTest.junit.xml
../source/MotherForker ./ThreadContainerTest --gtest_output=xml:../ThreadContainerTest.junit.xml
../source/MotherForker ./RESTBuilderTest --gtest_output=xml:../RESTBuilderTest.junit.xml
../source/MotherForker ./JSONEncoderTest --gtest_output=xml:../JSONEncoderTest.junit.xml
../source/MotherForker ./ConversionTest --gtest_output=xml:../ConversionTest.junit.xml
../source/MotherForker ./DiskPacketCaptureTest --gtest_output=xml:../DiskPacketCaptureTest.junit.xml
../source/MotherForker ./DiskCleanupTest --gtest_output=xml:../DiskCleanupTest.junit.xml
../source/MotherForker ./DiskUsageTest --gtest_output=xml:../DiskUsageTest.junit.xml
../source/MotherForker ./ElasticSearchTest --gtest_output=xml:../ElasticSearchTest.junit.xml
../source/MotherForker ./BoomStickTest --gtest_output=xml:../BoomStickTest.junit.xml
../source/MotherForker ./SyslogReporterTest --gtest_output=xml:../SyslogReporterTest.junit.xml


