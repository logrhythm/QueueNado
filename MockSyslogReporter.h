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
           mSyslogEnabled(true), mySyslogCmdSendToRestart(false),
           mTvSec(0),
           mTvUSec(0),
           mStatCount(0) {};

      void Start() LR_OVERRIDE {
         return SyslogReporter::Start();
      }

      void Join() LR_OVERRIDE{
         SyslogReporter::Join();
      }

      bool SyslogEnabled() LR_OVERRIDE {
         return mSyslogEnabled;
      }

      bool InitializeRsyslog() LR_OVERRIDE {
         if (mySyslogCmdSendToRestart) {
            return SyslogReporter::InitializeRsyslog();
         } else  {
            return false;
         }
      }

     void SetSyslogProtoConf(const protoMsg::SyslogConf& msg) {
        mSyslogCmdConf = msg;
     }
      
      protoMsg::SyslogConf  GetSyslogProtoConf() LR_OVERRIDE {
          return mSyslogCmdConf;
      }

      void SetSyslogCmdSendToRestart() {
         mySyslogCmdSendToRestart = true;
      }
       
      void SetSyslogEnabled(bool val) {
         mSyslogEnabled = val;
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


   private:
      bool mSyslogEnabled;
      bool mySyslogCmdSendToRestart;
      time_t mTvSec;
      time_t mTvUSec;
      unsigned int mStatCount;
      protoMsg::SyslogConf  mSyslogCmdConf;

};

