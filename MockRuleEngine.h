/* 
 * File:   MockRuleEngine.h
 * Author: Robert Weber
 *
 * Created on February 7, 2013, 8:23 AM
 */
#pragma once

#include "RuleEngine.h"
#ifdef LR_DEBUG 
namespace networkMonitor {

   class MockRuleEngine : public RuleEngine {
   public:

      MockRuleEngine(ConfSlave& slave, std::string& name, int option,
              int facility, int priority, bool master,
              unsigned int threadNumber) : RuleEngine(slave, name, option, facility,
      priority, master, threadNumber), mSiemMode(false), mSyslogEnabled(true),
      mMaxLineLength(2048), mScriptsDir("../scripts"),
      mStatsQueueName("ipc:///tmp/statsAccumulatorQ.ipc"),
      mDpiRcvrQueue("ipc:///tmp/dpilrmsgtest.ipc"),
      mDpiMsgQueueSize(1000), mSiemDebugMode(false) {
      };

      bool GetSyslogMessages(IndexedFieldPairs& formattedFieldData, std::vector<std::string>& messages, unsigned int dynamicStart) {
         return RuleEngine::GetSyslogMessages(formattedFieldData, messages, dynamicStart);
      }
      unsigned int GetApplicationSpecificFieldPairs(const unsigned int nextField,
           const DpiMsgLR& dpiMsg, IndexedFieldPairs& formattedFieldData) {
         return RuleEngine::GetApplicationSpecificFieldPairs(nextField,dpiMsg,formattedFieldData);
      }
      std::vector< std::string> GetSiemSyslogMessage(const DpiMsgLR& dpiMsg) {
         return RuleEngine::GetSiemSyslogMessage(dpiMsg);
      }

      std::string GetStaticInfo(const IndexedFieldPairs& formattedFieldData,const  unsigned int dynamicStart) {
         return RuleEngine::GetStaticInfo(formattedFieldData, dynamicStart);
      }

      std::string GetNextDataPair(std::map<unsigned int, std::pair<std::string, std::string> >::iterator& inputPointer) {
         return RuleEngine::GetNextDataPair(inputPointer);
      }

      unsigned int GetSiemRequiredFieldPairs(const DpiMsgLR& dpiMsg, IndexedFieldPairs& formattedFieldData) {
         return RuleEngine::GetSiemRequiredFieldPairs(dpiMsg, formattedFieldData);
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
         return RuleEngine::GetLoginField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetDomainField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetDomainField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetUrlField(const unsigned int nextField,const  DpiMsgLR& dpiMsg,const unsigned int threshold,IndexedFieldPairs& formattedFieldData)override {
         return RuleEngine::GetUrlField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetDestHostField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetDestHostField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetCommandField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetCommandField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetSenderField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetSenderField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetRecipientField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetRecipientField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetSubjectField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetSubjectField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetVersionField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetVersionField(nextField, dpiMsg, threshold,formattedFieldData);
      }
      unsigned int GetSessionField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetSessionField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetPathField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override{
         return RuleEngine::GetPathField(nextField, dpiMsg, threshold,formattedFieldData);
      }

      unsigned int GetFilenameField(const unsigned int nextField,const  DpiMsgLR& dpiMsg, const unsigned int threshold,IndexedFieldPairs& formattedFieldData) override {
         return RuleEngine::GetFilenameField(nextField, dpiMsg, threshold,formattedFieldData);
      }
      void RestartSyslogDaemon() {
         
      }
      void SetElasticSearchTarget(const std::string& target) {
         RuleEngine::mElasticSearchTarget = target;
         RuleEngine::mTransferElasticSearch.SetBinding(target);
      }
      bool mSiemMode;
      bool mSyslogEnabled;
      unsigned int mMaxLineLength;
      std::string mScriptsDir;
      std::string mStatsQueueName;
      std::string mDpiRcvrQueue;
      int mDpiMsgQueueSize;
      bool mSiemDebugMode;

   };

}
#endif