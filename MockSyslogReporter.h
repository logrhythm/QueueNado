/* 
 * File:   MockSyslogReporter.h
 * Author: John Gress
 *
 */
#pragma once

#include "SyslogReporter.h"
#include "SyslogConfMsg.pb.h"
#include "g2log.hpp"

class MockSyslogReporter : public SyslogReporter {
public:

   MockSyslogReporter(ConfSlave& slave, const std::string& name,
           const int option, const int facility, const int priority)
   : SyslogReporter(slave, name, option, facility, priority),
   mOverrideStatReportIntervalInSec(false), mOverrideStatIntervalInSec(0),
   mSyslogEnabled(true), mySyslogCmdSendToRestart(false),
   mStatCount(0) {
   };

   void Start() LR_OVERRIDE {
      return SyslogReporter::Start();
   }

   void Join() LR_OVERRIDE {
      SyslogReporter::Join();
   }

   bool SyslogEnabled() LR_OVERRIDE {
      return mSyslogEnabled;
   }

   bool InitializeRsyslog() LR_OVERRIDE {
      if (mySyslogCmdSendToRestart) {
         return SyslogReporter::InitializeRsyslog();
      } else {
         return false;
      }
   }

   void SetSyslogProtoConf(const protoMsg::SyslogConf& msg) {
      mSyslogCmdConf = msg;
   }

   protoMsg::SyslogConf GetSyslogProtoConf() LR_OVERRIDE {
      return mSyslogCmdConf;
   }

   void SetSyslogCmdSendToRestart() {
      mySyslogCmdSendToRestart = true;
   }

   void SetSyslogEnabled(bool val) {
      mSyslogEnabled = val;
   }

   bool IsSendStatTime() LR_OVERRIDE {
      return SyslogReporter::IsSendStatTime();
   }

   unsigned long GetStatReportIntervalInSec() LR_OVERRIDE {
      if (mOverrideStatReportIntervalInSec) {
         return mOverrideStatIntervalInSec;
      }
      return SyslogReporter::GetStatReportIntervalInSec();
   }

   void SendAccumStat(unsigned int stat) {
      mStatCount = stat;
   }

   void ResetConf() LR_OVERRIDE {
      SyslogReporter::ResetConf();
   }

   unsigned int GetStatCount() {
      return mStatCount;
   }

   void ResetStatCount() {
      mStatCount = 0;
   }


   bool mOverrideStatReportIntervalInSec;
   unsigned long mOverrideStatIntervalInSec;

private:
   bool mSyslogEnabled;
   bool mySyslogCmdSendToRestart;
   unsigned int mStatCount;
   protoMsg::SyslogConf mSyslogCmdConf;

};

