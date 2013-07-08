#!/bin/sh
chmod 777 .libs
chmod 777 .libs/* 
chmod 777 .
chmod 777 ..
chmod 777 ../source/.libs
chmod 777 ../source/.libs/*
touch ../ZMQTest.junit.xml ../PacketCaptureTest.junit.xml ../DpiMsgLRTest.junit.xml ../QosmosCodeGeneratorTest.junit.xml ../QosmosDpiTest.junit.xml ../SysLoggerTest.junit.xml ../SyslogConfigTest.junit.xml ../ConfProcessorTest.junit.xml ../SendStatsTest.junit.xml ../StatsAccumulatorTest.junit.xml ../ShotgunAlienTests.junit.xml ../RifleVampireTests.junit.xml ../TestTCPIPDirectionlessHash.junit.xml ../RuleEngineTest.junit.xml ../CommandProcessorTests.junit.xml ../ProcessManagerTest.junit.xml ../LuaExecuterTest.junit.xml ../ThreadContainerTest.junit.xml ../../RESTBuilderTest.junit.xml ../JSONEncodeTest.junit.xml ../ConversionTest.junit.xml ../DiskPacketCaptureTest.junit.xml ../DiskCleanupTest.junit.xml 
chmod 777 ../*.xml
./ZMQTest --gtest_output=xml:../ZMQTest.junit.xml
./PacketCaptureTest --gtest_output=xml:../PacketCaptureTest.junit.xml
./DpiMsgLRTest --gtest_output=xml:../DpiMsgLRTest.junit.xml
./QosmosCodeGeneratorTest --gtest_output=xml:../QosmosCodeGeneratorTest.junit.xml
./QosmosDpiTest --gtest_output=xml:../QosmosDpiTest.junit.xml
./SysLoggerTest --gtest_output=xml:../SysLoggerTest.junit.xml
./SyslogConfigTest --gtest_output=xml:../SyslogConfigTest.junit.xml
./ConfProcessorTests --gtest_output=xml:../ConfProcessorTest.junit.xml
./SendStatsTest --gtest_output=xml:../SendStatsTest.junit.xml
./StatsAccumulatorTest --gtest_output=xml:../StatsAccumulatorTest.junit.xml
./ShotgunAlienTests --gtest_output=xml:../ShotgunAlienTests.junit.xml
./RifleVampireTests --gtest_output=xml:../RifleVampireTests.junit.xml
./RuleEngineTest --gtest_output=xml:../RuleEngineTest.junit.xml
./TestTCPIPDirectionlessHash --gtest_output=xml:../TestTCPIPDirectionlessHash.junit.xml
./CommandProcessorTests --gtest_output=xml:../CommandProcessorTests.junit.xml
./ProcessManagerTest --gtest_output=xml:../ProcessManagerTest.junit.xml
./LuaExecuterTest --gtest_output=xml:../LuaExecuterTest.junit.xml
./ThreadContainerTest --gtest_output=xml:../ThreadContainerTest.junit.xml
./RESTBuilderTest --gtest_output=xml:../RESTBuilderTest.junit.xml
./JSONEncodeTest --gtest_output=xml:../JSONEncodeTest.junit.xml
./ConversionTest --gtest_output=xml:../ConversionTest.junit.xml
./DiskPacketCaptureTest --gtest_output=xml:../DiskPacketCaptureTest.junit.xml
./DiskCleanupTest --gtest_output=xml:../DiskCleanupTest.junit.xml


