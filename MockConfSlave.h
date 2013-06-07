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

      MockConfSlave() : mAppClosed(false), mNewConfSeen(false), mNewQosmosSeen(false), mNewSyslogSeen(false), mNewNetInterfaceMsg(false),
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
              const std::vector<std::string>& shots) {
         return mNewConfSeen = ConfSlave::ProcessConfMsg(configTypeMessage, shots);
      }

      bool ProcessQosmosMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots) {
         LOG(DEBUG) << "MockProcessQosmosMsg";
         return mNewQosmosSeen = ConfSlave::ProcessQosmosMsg(configTypeMessage, shots);
      }

      bool ProcessSyslogMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots) {
         LOG(DEBUG) << "MockProcessSyslogMsg";
         return mNewSyslogSeen = ConfSlave::ProcessSyslogMsg(configTypeMessage, shots);
      }

      bool ProcessRestartMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots) {
         LOG(DEBUG) << "MockProcessRestartMsg";
         return ConfSlave::ProcessRestartMsg(configTypeMessage, shots);
      }

      virtual bool ProcessNetInterfaceMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots) {
         LOG(DEBUG) << "MockProcessInterfaceMsg";
         return mNewNetInterfaceMsg = ConfSlave::ProcessNetInterfaceMsg(configTypeMessage, shots);
      }

      bool mAppClosed;
      bool mNewConfSeen;
      bool mNewQosmosSeen;
      bool mNewSyslogSeen;
      bool mNewNetInterfaceMsg;
      std::string mBroadcastQueueName;
#ifdef LR_DEBUG
      MockConf mConf;
#else
      Conf mConf;
#endif
   };
}

