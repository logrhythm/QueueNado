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
           mSyslogEnabled(true) {};

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

   private:
      bool mSyslogEnabled;

};

