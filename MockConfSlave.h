/* 
 * Author: Robert Weber
 *
 * Created on October 30, 2012, 8:18 AM
 */
#pragma once
#include "ConfSlave.h"
#include "MockConf.h"
#include "g2log.hpp"

namespace networkMonitor {

   class MockConfSlave : public ConfSlave {
   public:

      MockConfSlave() : mAppClosed(false), mNewConfSeen(false),
      mNewQosmosSeen(false), mNewSyslogSeen(false), mNewNetInterfaceMsg(false), mNewNtpMsg(false),
      mBroadcastQueueName("ipc:///tmp/testconfbroacast.ipc") {

      }

      Conf GetConf(void) {
         if (mConfLocation.empty()) {
            return mConf;
         } else {
            return ConfSlave::GetConf(mConfLocation);
         }
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
      
      virtual bool ProcessNtpMsg(const protoMsg::ConfType& configTypeMessage,
              const std::vector<std::string>& shots) {
         LOG(DEBUG) << "MockProcessNtpMsg";
         return mNewNtpMsg = ConfSlave::ProcessNtpMsg(configTypeMessage, shots);
      }

      bool mAppClosed;
      bool mNewConfSeen;
      bool mNewQosmosSeen;
      bool mNewSyslogSeen;
      bool mNewNetInterfaceMsg;
      bool mNewNtpMsg;
      std::string mBroadcastQueueName;
      std::string mConfLocation;
#ifdef LR_DEBUG
      MockConf mConf;
#else
      Conf mConf;
#endif
   };
}

