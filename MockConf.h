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
   mOverrideGetPcapCaptureLocations(true),
   mOverrideGetFirstCaptureLocation(true),
   mSyslogEnabled(true),
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

   mMaxIndividualPCap(1000),
   mPcapCaptureMaxPackets(999999),
   mSyslogSendQueueSize(800),
   mSyslogRecvQueueSize(800),
   mFlowReportInterval(10),
   mOverridegetPCapInterface(true) {
   }

   MockConf(const Conf& conf) : Conf::Conf(conf) {
   }

   MockConf(const protoMsg::BaseConf& msg, const QosmosConf& qosmosMsg, const protoMsg::SyslogConf& sysMsg) : Conf::Conf(msg, qosmosMsg, sysMsg) {
   }

   explicit MockConf(const std::string& path) : Conf::Conf(path) {
   }

   MockConf(std::stringstream& stream, std::stringstream& qosmosStream, std::stringstream& syslogStream) : Conf::Conf(stream, qosmosStream, syslogStream) {
   }

   ~MockConf() {
   }

   void writeSyslogToFile() {
      Conf::writeSyslogToFile();
   }

   bool sendConfigUpdate() {
      return Conf::sendConfigUpdate();
   }

   void writeQosmosToStream(std::stringstream & stream) const {
      Conf::writeQosmosToStream(stream);
   }

   void setPath(const std::string& newPath) {
      Conf::setPath(newPath);
   }

   void updateFields(const protoMsg::BaseConf& msg) {
      Conf::updateFields(msg);
   }

   void updateFields(const protoMsg::SyslogConf& msg) {
      Conf::updateFields(msg);
   }

   void updateQosmos(const QosmosConf& msg) {
      Conf::updateQosmos(msg);
   }

   std::string GetSyslogAgentIP(void) const LR_OVERRIDE {
      return mSyslogAgentIp;
   }

   bool GetSyslogEnabled(void) LR_OVERRIDE {
      return mSyslogEnabled;
   }

   std::string GetLogDir() const LR_OVERRIDE {
      return mLogDir;
   }

   std::string GetSyslogAgentPort(void) const LR_OVERRIDE {
      return mSyslogAgentPort;
   }

   std::string GetSyslogFacility(void) const LR_OVERRIDE {
      return mSyslogFacility;
   }

   std::string GetSyslogLogName(void) const LR_OVERRIDE {
      return mSyslogName;
   }

   std::string GetSyslogConfigFile(void) const LR_OVERRIDE {
      return mSyslogConfName;
   }

   bool GetSyslogTcpEnabled(void) const LR_OVERRIDE {
      return mSyslogProtocol;
   }

   std::string GetConfChangeQueue(void) const LR_OVERRIDE {
      return mConfChangeQueue;
   }

   std::string GetDpiRcvrQueue(void) LR_OVERRIDE {
      return mDpiRcvrQueue;
   }

   std::string GetBroadcastQueue(void) LR_OVERRIDE {
      return mBroadcastQueue;
   }

   std::string GetStatsAccumulatorQueue(void) const LR_OVERRIDE {
      return mStatsAccumulatorQueue;
   }

   std::string GetSendStatsQueue(void) LR_OVERRIDE {
      return mSendStatsQueue;
   }

   std::string GetPath(void) LR_OVERRIDE {
      return mPath;
   }

   unsigned int GetDpiThreads() LR_OVERRIDE {
      return mDpiThreads;
   }

   unsigned int GetPCAPETimeOut() LR_OVERRIDE {
      return mPCAPETimeOut;
   }

   unsigned int GetQosmosExpirePerCallback() LR_OVERRIDE {
      return mQosmosExpirePerCallback;
   }

   bool GetEnableTCPReassembly() LR_OVERRIDE {
      return mEnableTCPReAssembly;
   }

   bool GetEnableIPDefragmentation() LR_OVERRIDE {
      return mEnableIPDefragmentation;
   }

   bool GetSiemLogging() LR_OVERRIDE {
      return mSiemLogging;
   }

   int GetPCAPBuffsize() LR_OVERRIDE {
      return mPCAPBuffsize;
   }

   int GetPacketSendQueueSize() LR_OVERRIDE {
      return mPacketSendQueueSize;
   }

   int GetPacketRecvQueueSize() LR_OVERRIDE {
      return mPacketRecvQueueSize;
   }

   unsigned int GetSyslogMaxLineLength() LR_OVERRIDE {
      return mSyslogMaxLineLength;
   }

   int GetDPIMsgSendQueueSize() LR_OVERRIDE {
      return mDPIMsgSendQueueSize;
   }

   int GetSyslogSendQueueSize() LR_OVERRIDE {
      return mSyslogSendQueueSize;
   }

   unsigned int GetStatsIntervalSeconds() LR_OVERRIDE {
      return mStatsIntervalSeconds;
   }

   int GetDPIMsgRecvQueueSize() LR_OVERRIDE {
      return mDPIMsgRecvQueueSize;
   }

   int GetSyslogRecvQueueSize() LR_OVERRIDE {
      return mSyslogRecvQueueSize;
   }

   std::string GetPCAPInterface() LR_OVERRIDE {
      if (mOverridegetPCapInterface) {
         return mPCAPInterface;
      }
      return Conf::GetPCAPInterface();
   }

   bool GetQosmosDebugModeEnabled(void) LR_OVERRIDE {
      return mQosmosDebug;
   }

   unsigned int GetDpiHalfSessions() LR_OVERRIDE {
      return mDpiHalfSessions;
   }

   unsigned int GetQosmos64BytePool() LR_OVERRIDE {
      return mQosmos64;
   }

   unsigned int GetQosmos128BytePool() LR_OVERRIDE {
      return mQosmos128;
   }

   unsigned int GetQosmos256BytePool() LR_OVERRIDE {
      return mQosmos256;
   }

   unsigned int GetQosmos512BytePool() LR_OVERRIDE {
      return mQosmos512;
   }

   void InsertFakeQosmosProtocol(const std::string& name) {

      mQosmosProtoEnabled[name] = "false";
      mQosmosProtoLongNames[name] = name;
      mQosmosProtoFamiles[name] = name;
   }

   std::string GetCommandQueue() const LR_OVERRIDE {
      if (mCommandQueue.empty()) {
         return Conf::GetCommandQueue();
      }
      return mCommandQueue;
   }

   std::string GetProccessManagementQueue() const LR_OVERRIDE {
      if (mProcessManagmentQueue.empty()) {
         return Conf::GetProccessManagementQueue();
      }
      return mProcessManagmentQueue;
   }

   bool GetIntermediateFlowEnabled() LR_OVERRIDE {
      return mIntermediateFlowEnabled;
   }

   bool GetPacketCaptureEnabled() LR_OVERRIDE {
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

   std::vector<std::string> GetPcapCaptureLocations() const LR_OVERRIDE {
      if (mOverrideGetPcapCaptureLocations) {
         return mPCapCaptureLocations;
      }

      return Conf::GetPcapCaptureLocations();
   }

   std::string GetFirstPcapCaptureLocation() const LR_OVERRIDE {
      if (!mOverrideGetFirstCaptureLocation) {
         return Conf::GetFirstPcapCaptureLocation();
      }

      if (mPCapCaptureLocations.empty()) {
         return
         {
         };
      }

      return mPCapCaptureLocations[0];
   }

   size_t GetFlowReportInterval() LR_OVERRIDE {
      return mFlowReportInterval;

   }

   size_t GetPCapIndividualFileLimit() {
      return mMaxIndividualPCap;
   }

   size_t GetPcapCaptureMaxPackets() LR_OVERRIDE {
      return mPcapCaptureMaxPackets;
   }
   void ReadQosmosFromStringStream(std::stringstream& stream) {
      return Conf::ReadQosmosFromStringStream(stream);
   }
   ConfMap GetProtoMap() {
      return mConfMap;
   }
   void ClearProtoMap() {
      mConfMap.clear();
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
   std::vector<std::string> mPCapCaptureLocations;
   bool mOverrideGetPcapCaptureLocations;
   bool mOverrideGetFirstCaptureLocation;
   bool mSyslogEnabled;
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
   size_t mMaxIndividualPCap;
   size_t mPcapCaptureMaxPackets;
   int mSyslogSendQueueSize;
   int mSyslogRecvQueueSize;
   int mFlowReportInterval;
   bool mOverridegetPCapInterface;
};

/**
 * Allows you to use Update functions without a working ConfMaster
 */
class MockConfNoMaster : public MockConf {
public:

   MockConfNoMaster(const Conf& conf) : MockConf::MockConf(conf) {
   }

   MockConfNoMaster(const protoMsg::BaseConf& msg, const QosmosConf& qosmosMsg, const protoMsg::SyslogConf& sysMsg) : MockConf::MockConf(msg, qosmosMsg, sysMsg) {
   }

   explicit MockConfNoMaster(const std::string& path) : MockConf::MockConf(path) {
   }

   MockConfNoMaster(std::stringstream& stream, std::stringstream& qosmosStream, std::stringstream& syslogStream) : MockConf::MockConf(stream, qosmosStream, syslogStream) {
   }

   ~MockConfNoMaster() {
   }

   bool UpdateConfigWithMaster(const protoMsg::SyslogConf& msg) {
      updateFields(msg);
      return true;
   }

   bool UpdateConfigWithMaster(const protoMsg::BaseConf& msg) {
      updateFields(msg);
      return true;
   }

   bool UpdateConfigWithMaster(const QosmosConf& msg) {
      updateQosmos(msg);
      return true;

   }

   void ReadQosmosFromStringStream(std::stringstream& stream) {
      return Conf::ReadQosmosFromStringStream(stream);
   }
};

/**
 * A mock class that simply exposes protected methods 
 */
class MockConfExposeUpdate : public Conf {
public:

   MockConfExposeUpdate() : Conf() {
   }

   MockConfExposeUpdate(const Conf& conf) : Conf::Conf(conf) {
   }

   MockConfExposeUpdate(const protoMsg::BaseConf& msg, const QosmosConf& qosmosMsg, const protoMsg::SyslogConf& sysMsg) : Conf::Conf(msg, qosmosMsg, sysMsg) {
   }

   explicit MockConfExposeUpdate(const std::string& path) : Conf::Conf(path) {
   }

   MockConfExposeUpdate(std::stringstream& stream, std::stringstream& qosmosStream, std::stringstream& syslogStream) : Conf::Conf(stream, qosmosStream, syslogStream) {
   }

   ~MockConfExposeUpdate() {
   }

   void updateFields(const protoMsg::BaseConf& msg) {
      Conf::updateFields(msg);
   }

   void updateFields(const protoMsg::SyslogConf& msg) {
      Conf::updateFields(msg);
   }

   void updateQosmos(const QosmosConf& msg) {
      Conf::updateQosmos(msg);
   }

   void setPath(const std::string& newPath) {
      Conf::setPath(newPath);
   }

   void writeSyslogToFile() {
      Conf::writeSyslogToFile();
   }

   bool sendConfigUpdate() {
      return Conf::sendConfigUpdate();
   }

   void ReadQosmosFromStringStream(std::stringstream& stream) {
      return Conf::ReadQosmosFromStringStream(stream);
   }
};