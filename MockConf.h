/* 
 * File:   MockConf.h
 *
 * Created on December 26, 2012, 10:39 AM
 */

#pragma once
#include "Conf.h"
#include "include/global.h"
#ifdef LR_DEBUG

class MockConf : public Conf {
public:
   
   MockConf() :
   mSyslogAgentPort("1234"),
   mSyslogFacility("local4"),
   mDPIMsgSendQueueSize(1024),
   mDPIMsgRecvQueueSize(1024),
   mQosmosDebug(false),
   mDpiHalfSessions(0),
   mQosmos64(100000),
   mQosmos128(100000),
   mQosmos256(100000),
   mQosmos512(100000),
   mSiemDebug(false),
   mIntermediateFlowEnabled(false),
   mUnknownCaptureEnabled(false),
   mPCapCaptureFileLimit(0),
   mPCapCaptureSizeLimit(0),
   mPCapCaptureMemoryLimit(0),
   mSyslogEnabled(true),
   mReportEverything(false),
   mDpiThreads(3),
   mPCAPETimeOut(12),
   mQosmosExpirePerCallback(1),
   mEnableTCPReAssembly(true),
   mEnableIPDefragmentation(true),
   mSiemLogging(false),
   mPCAPBuffsize(1234),
   mPacketSendQueueSize(100),
   mPacketRecvQueueSize(100),
   mSyslogMaxLineLength(2048),
   mStatsIntervalSeconds(5) {
   }

   ~MockConf() {
   }

   std::string getSyslogAgentIP(void) override {
      return mSyslogAgentIp;
   }

   bool getSyslogEnabled(void) override {
      return mSyslogEnabled;
   }

   std::string getLogDir() const override {
      return mLogDir;
   }

   std::string getSyslogAgentPort(void) override {
      return mSyslogAgentPort;
   }

   std::string getSyslogFacility(void) override {
      return mSyslogFacility;
   }

   std::string getSyslogLogName(void) override {
      return mSyslogName;
   }

   std::string getSyslogConfName(void) override {
      return mSyslogConfName;
   }

   std::string getConfChangeQueue(void) override {
      return mConfChangeQueue;
   }

   std::string getDpiRcvrQueue(void) override {
      return mDpiRcvrQueue;
   }

   std::string getBroadcastQueue(void) override {
      return mBroadcastQueue;
   }

   std::string getStatsAccumulatorQueue(void) override {
      return mStatsAccumulatorQueue;
   }

   std::string getSendStatsQueue(void) override {
      return mSendStatsQueue;
   }

   std::string getPath(void) override {
      return mPath;
   }

   bool getReportEveythingEnabled(void) override {
      return mReportEverything;
   }

   unsigned int getDpiThreads() override {
      return mDpiThreads;
   }

   unsigned int getPCAPETimeOut() override {
      return mPCAPETimeOut;
   }

   unsigned int getQosmosExpirePerCallback() override {
      return mQosmosExpirePerCallback;
   }

   bool EnableTCPReassembly() override {
      return mEnableTCPReAssembly;
   }

   bool EnableIPDefragmentation() override {
      return mEnableIPDefragmentation;
   }

   bool SiemLogging() override {
      return mSiemLogging;
   }

   int getPCAPBuffsize() override {
      return mPCAPBuffsize;
   }

   int GetPacketSendQueueSize() override {
      return mPacketSendQueueSize;
   }

   int GetPacketRecvQueueSize() override {
      return mPacketRecvQueueSize;
   }

   unsigned int getSyslogMaxLineLength() override {
      return mSyslogMaxLineLength;
   }

   int GetDPIMsgSendQueueSize() override {
      return mDPIMsgSendQueueSize;
   }

   unsigned int getStatsIntervalSeconds() override {
      return mStatsIntervalSeconds;
   }

   int GetDPIMsgRecvQueueSize() override {
      return mDPIMsgRecvQueueSize;
   }

   std::string getPCAPInterface() override {
      return mPCAPInterface;
   }

   bool getQosmosDebugModeEnabled(void) override {
      return mQosmosDebug;
   }

   unsigned int getDpiHalfSessions() override {
      return mDpiHalfSessions;
   }

   unsigned int getQosmos64BytePool() override {
      return mQosmos64;
   }

   unsigned int getQosmos128BytePool() override {
      return mQosmos128;
   }

   unsigned int getQosmos256BytePool() override {
      return mQosmos256;
   }

   unsigned int getQosmos512BytePool() override {
      return mQosmos512;
   }

   void InsertFakeQosmosProtocol(const std::string& name) {

      mQosmosProtoEnabled[name] = "false";
      mQosmosProtoLongNames[name] = name;
      mQosmosProtoFamiles[name] = name;
   }

   std::string getCommandQueue() const override {
      if (mCommandQueue.empty()) {
         return Conf::getCommandQueue();
      }
      return mCommandQueue;
   }

   std::string getProccessManagementQueue() const override {
      if (mProcessManagmentQueue.empty()) {
         return Conf::getProccessManagementQueue();
      }
      return mProcessManagmentQueue;
   }

   bool IntermediateFlowEnabled() override {
      return mIntermediateFlowEnabled;
   }

   bool UnknownCaptureEnabled() override {
      return mUnknownCaptureEnabled;
   }

   size_t GetPcapCaptureFileLimit() override {
      return mPCapCaptureFileLimit;
   }

   size_t GetPcapCaptureSizeLimit() override {
      return mPCapCaptureSizeLimit;
   }

   int GetPcapCaptureMemoryLimit() override {
      return mPCapCaptureMemoryLimit;
   }

   std::string GetPcapCaptureLocation() override {
      return mPCapCaptureLocation;
   }

   bool SiemDebugLogging() {
      return mSiemDebug;
   }
   std::string mSyslogAgentPort;
   std::string mSyslogFacility;
   std::string mSyslogName;
   std::string mSyslogConfName;
   std::string mSyslogAgentIp;
   std::string mLogDir;
   std::string mConfChangeQueue;
   std::string mDpiRcvrQueue;
   std::string mBroadcastQueue;
   std::string mStatsAccumulatorQueue;
   std::string mSendStatsQueue;
   std::string mPath;
   std::string mCommandQueue;
   std::string mProcessManagmentQueue;
   std::string mPCapCaptureLocation;
   std::string mPCAPInterface;
   int mDPIMsgSendQueueSize;
   int mDPIMsgRecvQueueSize;
   bool mQosmosDebug;
   unsigned int mDpiHalfSessions;
   unsigned int mQosmos64;
   unsigned int mQosmos128;
   unsigned int mQosmos256;
   unsigned int mQosmos512;
   bool mSiemDebug;
   bool mIntermediateFlowEnabled;
   bool mUnknownCaptureEnabled;
   int mPCapCaptureFileLimit;
   int mPCapCaptureSizeLimit;
   int mPCapCaptureMemoryLimit;
   bool mSyslogEnabled;
   bool mReportEverything;
   unsigned int mDpiThreads;
   unsigned int mPCAPETimeOut;
   unsigned int mQosmosExpirePerCallback;
   bool mEnableTCPReAssembly;
   bool mEnableIPDefragmentation;
   bool mSiemLogging;
   int mPCAPBuffsize;
   int mPacketSendQueueSize;
   int mPacketRecvQueueSize;
   unsigned int mSyslogMaxLineLength;
   unsigned int mStatsIntervalSeconds;

};
#endif