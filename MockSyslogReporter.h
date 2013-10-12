/* 
 * File:   MockSyslogReporter.h
 * Author: John Gress
 *
 */
#pragma once

#include "SyslogReporter.h"

class MockSyslogReporter : public SyslogReporter {

   public:
      MockSyslogReporter(ConfSlave& slave) : SyslogReporter(slave) {};

      boost::thread* Start() {
         return SyslogReporter::Start();
      }

      void Join() {
         SyslogReporter::Join();
      }

};

#if 0
#include "SyslogReporter.h"
#ifdef LR_DEBUG 
namespace networkMonitor {

   class MockSyslogReporter : public SyslogReporter {
   public:

      MockSyslogReporter(ConfSlave& slave, std::string& name, int option,
              int facility, int priority, bool master,
              unsigned int threadNumber) : SyslogReporter(slave, name, option, facility,
      priority, master, threadNumber), mSiemMode(false), mSyslogEnabled(true),
      mMaxLineLength(2048), mScriptsDir("../scripts"),
      mStatsQueueName("ipc:///tmp/statsAccumulatorQ.ipc"),
      mDpiRcvrQueue("ipc:///tmp/dpilrmsgtest.ipc"),
      mDpiMsgQueueSize(1000), mSiemDebugMode(false), mSentUpdate(false) {
      };

      bool GetSyslogMessages(IndexedFieldPairs& formattedFieldData, std::vector<std::string>& messages, unsigned int dynamicStart) {
         return SyslogReporter::GetSyslogMessages(formattedFieldData, messages, dynamicStart);
      }
      unsigned int GetApplicationSpecificFieldPairs(const unsigned int nextField,
           const DpiMsgLR& dpiMsg, IndexedFieldPairs& formattedFieldData) {
         return SyslogReporter::GetApplicationSpecificFieldPairs(nextField,dpiMsg,formattedFieldData);
      }
      std::vector< std::string> GetSiemSyslogMessage(const DpiMsgLR& dpiMsg) {
         return SyslogReporter::GetSiemSyslogMessage(dpiMsg);
      }

      std::string GetStaticInfo(const IndexedFieldPairs& formattedFieldData,const  unsigned int dynamicStart) {
         return SyslogReporter::GetStaticInfo(formattedFieldData, dynamicStart);
      }

      std::string GetNextDataPair(std::map<unsigned int, std::pair<std::string, std::string> >::iterator& inputPointer) {
         return SyslogReporter::GetNextDataPair(inputPointer);
      }

      unsigned int GetSiemRequiredFieldPairs(const DpiMsgLR& dpiMsg, IndexedFieldPairs& formattedFieldData) {
         return SyslogReporter::GetSiemRequiredFieldPairs(dpiMsg, formattedFieldData);
      }

      void SetMaxSize(const unsigned int max) {
         mMaxSyslogMsgSize = max;
      }

      bool SiemModeEnabled() {
         return mSiemMode;
      }
      bool SiemDebugModeEnabled() {
         return mSiemDebugMode;
      }
      bool SyslogEnabled() {
         return mSyslogEnabled;
      }

      unsigned int MaxLineLength() {
         return mMaxLineLength;
      }

      std::string GetScriptsDir() {
         return mScriptsDir;
      }

      std::string GetStatsAccQueue() {
         return mStatsQueueName;
      }

      std::string GetDpiRcvrQueue() {
         return mDpiRcvrQueue;
      }

      int GetDpiMsgQueueSize() {
         return mDpiMsgQueueSize;
      }

      unsigned int GetLoginField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetLoginField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetDomainField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetDomainField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetUrlField(const unsigned int nextField,const  DpiMsgLR& dpiMsg,const unsigned int threshold,IndexedFieldPairs& formattedFieldData)override {
         return SyslogReporter::GetUrlField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetDestHostField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetDestHostField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetCommandField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetCommandField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetSenderField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetSenderField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetRecipientField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetRecipientField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetSubjectField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetSubjectField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetVersionField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetVersionField(nextField, dpiMsg, threshold,formattedFieldData);
      }
      unsigned int GetSessionField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetSessionField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetPathField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return SyslogReporter::GetPathField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetFilenameField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override {
         return SyslogReporter::GetFilenameField(nextField, dpiMsg, threshold,formattedFieldData);
      }
      void RestartSyslogDaemon() {
         
      }
      void SetElasticSearchTarget(const std::string& target) {
         SyslogReporter::mElasticSearchTarget = target;
         SyslogReporter::mTransferElasticSearch.SetBinding(target);
      }
      void UpdatePreviousRecordNoLongerLatest(networkMonitor::DpiMsgLR* dpiMsg) {
         SyslogReporter::UpdatePreviousRecordNoLongerLatest(dpiMsg);
      }
      bool UpdateDocWithDpiMsg(const networkMonitor::DpiMsgLR& dpiMsg) {
         mEsMessage.Clear();
         mEsMessage = dpiMsg;
         mSentUpdate = true;
         return true;
      }
      bool AddDocWithDpiMsg(const networkMonitor::DpiMsgLR& dpiMsg) {
         mEsMessage.Clear();
         mEsMessage = dpiMsg;
         return true;
      }
      bool mSiemMode;
      bool mSyslogEnabled;
      unsigned int mMaxLineLength;
      std::string mScriptsDir;
      std::string mStatsQueueName;
      std::string mDpiRcvrQueue;
      int mDpiMsgQueueSize;
      bool mSiemDebugMode;
      bool mSentUpdate;
      DpiMsgLR mEsMessage;
   };

}
#endif
#endif
