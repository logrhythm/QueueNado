/* 
 * File:   MockSyslogReporter.h
 * Author: John Gress
 *
 */
#pragma once

#include "SyslogReporter.h"

class MockSyslogReporter : public SyslogReporter {

   public:
      MockSyslogReporter(ConfSlave& slave, const std::string& name,
              const int option, const int facility, const int priority)
         : SyslogReporter(slave, name, option, facility, priority),
           mSyslogEnabled(true),
           mTvSec(0),
           mTvUSec(0),
           mStatCount(0) {};

      boost::thread* Start() {
         return SyslogReporter::Start();
      }

      void Join() {
         SyslogReporter::Join();
      }

      bool SyslogEnabled() {
         return mSyslogEnabled;
      }

      void SetSyslogEnabled(bool val) {
         mSyslogEnabled = val;
      }

      void SendAccumStat(unsigned int stat) {
         mStatCount = stat;
      }

      unsigned int GetStatCount() {
         return mStatCount;
      }

      void ResetStatCount() {
         mStatCount = 0;
      }

      void GetTime(struct timeval& tv) {
         if ( mTvSec != 0 ) {
            tv.tv_sec = mTvSec;
            tv.tv_usec = mTvUSec;
         } else {
            SyslogReporter::GetTime(tv);
         }
      }

      void SetMockTime(time_t tvSec, time_t tvUSec) {
         mTvSec = tvSec;
         mTvUSec = tvUSec;
      }

   private:
      bool mSyslogEnabled;
      time_t mTvSec;
      time_t mTvUSec;
      unsigned int mStatCount;

};

