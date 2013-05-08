/* 
 * File:   MockConf.h
 *
 * Created on December 26, 2012, 10:39 AM
 */

#pragma once
#include "Conf.h"
#include "include/global.h"

class MockConf : public Conf {
public:

   MockConf() : mQosmosDebug(false), mDpiHalfSessions(0), mQosmos64(100000),
   mQosmos128(100000), mQosmos256(100000), mQosmos512(100000), mSyslogEnabled(true),
   mSyslogAgentPort("1234"), mSyslogFacility("local4"), mReportEverything(false),
   mDpiThreads(3), mLogBufferSize(124), mPCAPETimeOut(12), mQosmosExpirePerCallback(1),
   mSingleAppMultiThreadQosmos(true), mEnableTCPReAssembly(true), mEnableIPDefragmentation(true),
   mSiemLogging(false), mPCAPBuffsize(1234), mPacketSendQueueSize(100),
   mPacketRecvQueueSize(100), mSyslogMaxLineLength(2048), mDPIMsgSendQueueSize(1024),
   mStatsIntervalSeconds(5), mDPIMsgRecvQueueSize(1024), mSiemDebug(false), mIntermediateFlowEnabled(false),
   mUnknownCaptureEnabled(false), mPCapCaptureFileLimit(0), mPCapCaptureSizeLimit(0),
   mPCapCaptureMemoryLimit(0) {
   }

   ~MockConf() {
   }

   std::string getSyslogAgentIP(void) {
      return mSyslogAgentIp;
   }
   std::string mSyslogAgentIp;

   bool getSyslogEnabled(void) {
      return mSyslogEnabled;
   }
   bool mSyslogEnabled;

   std::string getLogDir() {
      return mLogDir;
   }
   std::string mLogDir;

   std::string getSyslogAgentPort(void) {
      return mSyslogAgentPort;
   }
   std::string mSyslogAgentPort;

   std::string getSyslogFacility(void) {
      return mSyslogFacility;
   }
   std::string mSyslogFacility;

   std::string getSyslogLogName(void) {
      return mSyslogName;
   }
   std::string mSyslogName;

   std::string getSyslogConfName(void) {
      return mSyslogConfName;
   }
   std::string mSyslogConfName;

   std::string getConfChangeQueue(void) {
      return mConfChangeQueue;
   }
   std::string mConfChangeQueue;

   std::string getDpiRcvrQueue(void) {
      return mDpiRcvrQueue;
   }
   std::string mDpiRcvrQueue;

   std::string getBroadcastQueue(void) {
      return mBroadcastQueue;
   }
   std::string mBroadcastQueue;

   std::string getStatsAccumulatorQueue(void) {
      return mStatsAccumulatorQueue;
   }
   std::string mStatsAccumulatorQueue;

   std::string getSendStatsQueue(void) {
      return mSendStatsQueue;
   }
   std::string mSendStatsQueue;

   std::string getPath(void) {
      return mPath;
   }
   std::string mPath;

   bool getReportEveythingEnabled(void) {
      return mReportEverything;
   }
   bool mReportEverything;

   unsigned int getDpiThreads() {
      return mDpiThreads;
   }
   unsigned int mDpiThreads;

   unsigned int getLogBufferSize() {
      return mLogBufferSize;
   }
   unsigned int mLogBufferSize;

   unsigned int getPCAPETimeOut() {
      return mPCAPETimeOut;
   }
   unsigned int mPCAPETimeOut;

   unsigned int getQosmosExpirePerCallback() {
      return mQosmosExpirePerCallback;
   }
   unsigned int mQosmosExpirePerCallback;

   bool SingleAppMultiThreadQosmos() {
      return mSingleAppMultiThreadQosmos;
   }
   bool mSingleAppMultiThreadQosmos;

   bool EnableTCPReassembly() {
      return mEnableTCPReAssembly;
   }
   bool mEnableTCPReAssembly;

   bool EnableIPDefragmentation() {
      return mEnableIPDefragmentation;
   }
   bool mEnableIPDefragmentation;

   bool SiemLogging() {
      return mSiemLogging;
   }
   bool mSiemLogging;

   int getPCAPBuffsize() {
      return mPCAPBuffsize;
   }
   int mPCAPBuffsize;

   int GetPacketSendQueueSize() {
      return mPacketSendQueueSize;
   }
   int mPacketSendQueueSize;

   int GetPacketRecvQueueSize() {
      return mPacketRecvQueueSize;
   }
   int mPacketRecvQueueSize;

   unsigned int getSyslogMaxLineLength() {
      return mSyslogMaxLineLength;
   }
   unsigned int mSyslogMaxLineLength;

   int GetDPIMsgSendQueueSize() {
      return mDPIMsgSendQueueSize;
   }
   int mDPIMsgSendQueueSize;

   unsigned int getStatsIntervalSeconds() {
      return mStatsIntervalSeconds;
   }
   unsigned int mStatsIntervalSeconds;

   int GetDPIMsgRecvQueueSize() {
      return mDPIMsgRecvQueueSize;
   }
   int mDPIMsgRecvQueueSize;

   std::string getPCAPInterface() {
      return mPCAPInterface;
   }
   std::string mPCAPInterface;

   bool getQosmosDebugModeEnabled(void) {
      return mQosmosDebug;
   }

   unsigned int getDpiHalfSessions() {
      return mDpiHalfSessions;
   }

   unsigned int getQosmos64BytePool() {
      return mQosmos64;
   }

   unsigned int getQosmos128BytePool() {
      return mQosmos128;
   }

   unsigned int getQosmos256BytePool() {
      return mQosmos256;
   }

   unsigned int getQosmos512BytePool() {
      return mQosmos512;
   }

   void InsertFakeQosmosProtocol(const std::string& name) {

      mQosmosProtoEnabled[name] = "false";
      mQosmosProtoLongNames[name] = name;
      mQosmosProtoFamiles[name] = name;
   }

   std::string getCommandQueue() const {
      if (mCommandQueue.empty()) {
         return Conf::getCommandQueue();
      }
      return mCommandQueue;
   }

   std::string getProccessManagementQueue() const {
      if (mProcessManagmentQueue.empty()) {
         return Conf::getProccessManagementQueue();
      }
      return mProcessManagmentQueue;
   }

   bool IntermediateFlowEnabled() {
      return mIntermediateFlowEnabled;
   }

   bool UnknownCaptureEnabled() {
      return mUnknownCaptureEnabled;
   }

   int GetPcapCaptureFileLimit() {
      return mPCapCaptureFileLimit;
   }

   int GetPcapCaptureSizeLimit() {
      return mPCapCaptureSizeLimit;
   }
   int GetPcapCaptureMemoryLimit() {
      return mPCapCaptureMemoryLimit;
   }
   std::string GetPcapCaptureLocation() {
      return mPCapCaptureLocation;
   }
   bool mQosmosDebug;
   unsigned int mDpiHalfSessions;
   unsigned int mQosmos64;
   unsigned int mQosmos128;
   unsigned int mQosmos256;
   unsigned int mQosmos512;

   bool SiemDebugLogging() {
      return mSiemDebug;
   }
   bool mSiemDebug;
   std::string mCommandQueue;
   std::string mProcessManagmentQueue;
   bool mIntermediateFlowEnabled;
   bool mUnknownCaptureEnabled;
   int mPCapCaptureFileLimit;
   int mPCapCaptureSizeLimit;
   std::string mPCapCaptureLocation;
   int mPCapCaptureMemoryLimit;
};
