/* 
 * File:   MockConf.h
 *
 * Created on December 26, 2012, 10:39 AM
 */

#pragma once
#include "Conf.h"
#include "include/global.h"
#include "Range.h"
#include <g2log.hpp>
#include <functional>
#include <exception>

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
   mPCapCaptureFileLimit(999999),
   mPCapCaptureSizeLimit(999999),
   mPCapCaptureMemoryLimit(999999),
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
   mStatsIntervalSeconds(5),
   mOverrideInternalRepair(false),
   mInternalRepair(true),
   mValidateEthFailCount(0),
   mMaxIndividualPCap(1000), 
   mValidConf(true),
   mIgnoreSyslogConfValidation(true),
   mIgnoreBaseConfValidation(true),
   mPcapCaptureMaxPackets(999999),
   mSyslogSendQueueSize(800),
   mSyslogRecvQueueSize(800) {
}

   ~MockConf() {
   }

   std::string getSyslogAgentIP(void) LR_OVERRIDE {
      return mSyslogAgentIp;
   }

   bool getSyslogEnabled(void) LR_OVERRIDE {
      return mSyslogEnabled;
   }

   std::string getLogDir() const LR_OVERRIDE {
      return mLogDir;
   }

   std::string getSyslogAgentPort(void) LR_OVERRIDE {
      return mSyslogAgentPort;
   }

   std::string getSyslogFacility(void) LR_OVERRIDE {
      return mSyslogFacility;
   }

   std::string getSyslogLogName(void) LR_OVERRIDE {
      return mSyslogName;
   }

   std::string getSyslogConfigFile(void) LR_OVERRIDE {
      return mSyslogConfName;
   }

   bool getSyslogTcpEnabled(void) LR_OVERRIDE {
      return mSyslogProtocol;
   }
   
   std::string getConfChangeQueue(void) LR_OVERRIDE {
      return mConfChangeQueue;
   }

   std::string getDpiRcvrQueue(void) LR_OVERRIDE {
      return mDpiRcvrQueue;
   }

   std::string getBroadcastQueue(void) LR_OVERRIDE {
      return mBroadcastQueue;
   }

   std::string getStatsAccumulatorQueue(void) LR_OVERRIDE {
      return mStatsAccumulatorQueue;
   }

   std::string getSendStatsQueue(void) LR_OVERRIDE {
      return mSendStatsQueue;
   }

   std::string getPath(void) LR_OVERRIDE {
      return mPath;
   }

   bool getReportEveythingEnabled(void) LR_OVERRIDE {
      return mReportEverything;
   }

   unsigned int getDpiThreads() LR_OVERRIDE {
      return mDpiThreads;
   }

   unsigned int getPCAPETimeOut() LR_OVERRIDE {
      return mPCAPETimeOut;
   }

   unsigned int getQosmosExpirePerCallback() LR_OVERRIDE {
      return mQosmosExpirePerCallback;
   }

   bool EnableTCPReassembly() LR_OVERRIDE {
      return mEnableTCPReAssembly;
   }

   bool EnableIPDefragmentation() LR_OVERRIDE {
      return mEnableIPDefragmentation;
   }

   bool SiemLogging() LR_OVERRIDE {
      return mSiemLogging;
   }

   int getPCAPBuffsize() LR_OVERRIDE {
      return mPCAPBuffsize;
   }

   int GetPacketSendQueueSize() LR_OVERRIDE {
      return mPacketSendQueueSize;
   }

   int GetPacketRecvQueueSize() LR_OVERRIDE {
      return mPacketRecvQueueSize;
   }

   unsigned int getSyslogMaxLineLength() LR_OVERRIDE {
      return mSyslogMaxLineLength;
   }

   int GetDPIMsgSendQueueSize() LR_OVERRIDE {
      return mDPIMsgSendQueueSize;
   }

   int GetSyslogSendQueueSize() LR_OVERRIDE {
      return mSyslogSendQueueSize;
   }

   unsigned int getStatsIntervalSeconds() LR_OVERRIDE {
      return mStatsIntervalSeconds;
   }

   int GetDPIMsgRecvQueueSize() LR_OVERRIDE {
      return mDPIMsgRecvQueueSize;
   }

   int GetSyslogRecvQueueSize() LR_OVERRIDE {
      return mSyslogRecvQueueSize;
   }

   std::string getPCAPInterface() LR_OVERRIDE {
      return mPCAPInterface;
   }

   bool getQosmosDebugModeEnabled(void) LR_OVERRIDE {
      return mQosmosDebug;
   }

   unsigned int getDpiHalfSessions() LR_OVERRIDE {
      return mDpiHalfSessions;
   }

   unsigned int getQosmos64BytePool() LR_OVERRIDE {
      return mQosmos64;
   }

   unsigned int getQosmos128BytePool() LR_OVERRIDE {
      return mQosmos128;
   }

   unsigned int getQosmos256BytePool() LR_OVERRIDE {
      return mQosmos256;
   }

   unsigned int getQosmos512BytePool() LR_OVERRIDE {
      return mQosmos512;
   }

   void InsertFakeQosmosProtocol(const std::string& name) {

      mQosmosProtoEnabled[name] = "false";
      mQosmosProtoLongNames[name] = name;
      mQosmosProtoFamiles[name] = name;
   }

   std::string getCommandQueue() const LR_OVERRIDE {
      if (mCommandQueue.empty()) {
         return Conf::getCommandQueue();
      }
      return mCommandQueue;
   }

   std::string getProccessManagementQueue() const LR_OVERRIDE {
      if (mProcessManagmentQueue.empty()) {
         return Conf::getProccessManagementQueue();
      }
      return mProcessManagmentQueue;
   }

   bool IntermediateFlowEnabled() LR_OVERRIDE {
      return mIntermediateFlowEnabled;
   }

   bool UnknownCaptureEnabled() LR_OVERRIDE {
      return mUnknownCaptureEnabled;
   }

   size_t GetPcapCaptureFileLimit() LR_OVERRIDE {
      return mPCapCaptureFileLimit;
   }

   size_t GetPcapCaptureSizeLimit() LR_OVERRIDE {
      return mPCapCaptureSizeLimit;
   }

   int GetPcapCaptureMemoryLimit() LR_OVERRIDE {
      return mPCapCaptureMemoryLimit;
   }

   std::string GetPcapCaptureLocation() LR_OVERRIDE {
      return mPCapCaptureLocation;
   }

   bool SiemDebugLogging() {
      return mSiemDebug;
   }

   bool InternallyRepairBaseConf(EthInfo& ethInfo) LR_OVERRIDE {
      if (mOverrideInternalRepair) {
         return mInternalRepair;
      }
      return Conf::InternallyRepairBaseConf(ethInfo);
   }

   void RepairEthConfFieldsWithDefaults(ConfMap& protoMap, EthInfo& ethInfo) LR_OVERRIDE {
      if (mValidateEthFailCount > 0) {
         mValidateEthFailCount--;
      }
      return Conf::RepairEthConfFieldsWithDefaults(protoMap, ethInfo);
   }

   bool ValidateEthConfFields(ConfMap& protoMap, EthInfo& ethInfo) LR_OVERRIDE {
      if (mValidateEthFailCount > 0) {
         return false;
      } else if (mValidateEthFailCount == 0) {
         return true;
      }
      mValidateEthFailCount = -1;
      return Conf::ValidateEthConfFields(protoMap, ethInfo);
   }


   size_t GetPCapIndividualFileLimit() {
      return mMaxIndividualPCap;
   }
   size_t GetPcapCaptureMaxPackets() LR_OVERRIDE {
      return mPcapCaptureMaxPackets;
   }

   bool ValidateBaseConf(protoMsg::BaseConf& msg) LR_OVERRIDE {
      if (mIgnoreBaseConfValidation) {
         return true;
      }
      mValidConf = Conf::ValidateBaseConf(msg);
      return mValidConf;
   }

   bool ValidateSyslogConf(protoMsg::SyslogConf& msg) LR_OVERRIDE {
      if (mIgnoreSyslogConfValidation) {
         return true;
      }
      mValidConf = Conf::ValidateSyslogConf(msg);
      return mValidConf;
   }


   bool CheckNumber(const std::string& number, const Range& range) LR_OVERRIDE {
         return Conf::CheckNumber(number, range);
   }
   
   void CheckNumberForNegative(const std::string& number) LR_OVERRIDE {
      try {
         Conf::CheckNumberForNegative(number);
         } catch (std::exception e) {
         LOG(DEBUG) << e.what();
         mValidConf = false;
         throw;
      }
      mValidConf = true;
   }

   void CheckNumberForSize(const std::string& number, const Range& range) LR_OVERRIDE {
      try {
         Conf::CheckNumberForSize(number, range);
         } catch (std::exception e) {
         LOG(DEBUG) << e.what();
         mValidConf = false;
         throw;
      }
      mValidConf = true;
   }

   bool CheckString(const std::string& text) LR_OVERRIDE {
      return Conf::CheckString(text);
   }

   void CheckStringForSize(const std::string& text) LR_OVERRIDE {
      try {
         Conf::CheckStringForSize(text);
      } catch (std::exception e) {
         LOG(DEBUG) << e.what();
         mValidConf = false;
         throw;
      }
      mValidConf = true;
   }
   
   bool CheckBool(const std::string& text) LR_OVERRIDE {
    mValidConf = Conf::CheckBool(text);
    return mValidConf;
   }

   std::string mSyslogAgentPort;
   std::string mSyslogFacility;
   std::string mSyslogName;
   std::string mSyslogConfName;
   bool mSyslogProtocol;
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
   bool mOverrideInternalRepair;
   bool mInternalRepair;
   int mValidateEthFailCount;

   size_t mMaxIndividualPCap;
   bool mValidConf;
   bool mIgnoreSyslogConfValidation;
   bool mIgnoreBaseConfValidation;
   size_t mPcapCaptureMaxPackets;
   int mSyslogSendQueueSize;
   int mSyslogRecvQueueSize;
};
