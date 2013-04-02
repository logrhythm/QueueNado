/* 
 * Author: Robert Weber
 *
 * Created on October 30, 2012, 8:18 AM
 */
#pragma once
#include "ConfSlave.h"
#include "MockConf.h"
#include "g2logworker.hpp"
#include "g2log.hpp"

namespace networkMonitor {
class MockConfSlave : public ConfSlave {
public:
   MockConfSlave() : mAppClosed(false), mNewConfSeen(false), mNewQosmosSeen(false), mNewSyslogSeen(false),
                     mBroadcastQueueName("ipc:///tmp/testconfbroacast.ipc") {
      
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
      return mNewConfSeen=ConfSlave::ProcessConfMsg(configTypeMessage,shots,conf);
   }
   bool ProcessQosmosMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots,
              Conf& conf) {
      LOG(DEBUG) << "MockProcessQosmosMsg";
      if (configTypeMessage.type() == protoMsg::ConfType_Type_QOSMOS  
              && shots.size() > 1 ) {
         mNewQosmosSeen=true;
      }
      return ConfSlave::ProcessQosmosMsg(configTypeMessage,shots,conf);
   }
   bool ProcessSyslogMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots,
              Conf& conf) {
      return mNewSyslogSeen=ConfSlave::ProcessSyslogMsg(configTypeMessage,shots,conf);
   }
   bool ProcessRestartMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots,
              Conf& conf) {
      return ConfSlave::ProcessRestartMsg(configTypeMessage,shots,conf);
   }
   bool mAppClosed;
   bool mNewConfSeen;
   bool mNewQosmosSeen;
   bool mNewSyslogSeen;
   std::string mBroadcastQueueName;
   MockConf mConf;
};
}

