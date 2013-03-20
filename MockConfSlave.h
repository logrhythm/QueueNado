/* 
 * Author: Robert Weber
 *
 * Created on October 30, 2012, 8:18 AM
 */
#pragma once
#include "ConfSlave.h"
#include "MockConf.h"

namespace networkMonitor {
class MockConfSlave : public ConfSlave {
public:
   MockConfSlave() : mAppClosed(false), mBroadcastQueueName("ipc:///tmp/testconfbroacast.ipc") {
      
   }
   Conf GetConf(void) {
      return mConf;
   }
   void EndApplication() {
      mAppClosed = true;
   }
   std::string GetBroadcastQueue() {
      return mBroadcastQueueName;
   }
   bool ProcessMessage(std::vector<std::string>& shots) {
      return ConfSlave::ProcessMessage(shots);
   }

   bool ProcessConfMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots,
              Conf& conf) {
      return ConfSlave::ProcessConfMsg(configTypeMessage,shots,conf);
   }
   bool ProcessQosmosMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots,
              Conf& conf) {
      return ConfSlave::ProcessQosmosMsg(configTypeMessage,shots,conf);
   }
   bool ProcessSyslogMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots,
              Conf& conf) {
      return ConfSlave::ProcessSyslogMsg(configTypeMessage,shots,conf);
   }
   bool ProcessRestartMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots,
              Conf& conf) {
      return ConfSlave::ProcessRestartMsg(configTypeMessage,shots,conf);
   }
   bool mAppClosed;
   std::string mBroadcastQueueName;
   MockConf mConf;
};
}

