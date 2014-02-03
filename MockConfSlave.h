/* 
 * Author: Robert Weber
 *
 * Created on October 30, 2012, 8:18 AM
 */
#pragma once
#include "ConfSlave.h"
#include "MockConf.h"
#include "g2log.hpp"
#include "ShutdownMsg.pb.h"

class MockConfSlave : public ConfSlave {
public:

   MockConfSlave() : mShutDownConfReceived(false), mShutDownConfValue(false), mAppClosed(false), mNewConfSeen(false),
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

   std::string GetBroadcastQueue() LR_OVERRIDE {
      return mBroadcastQueueName;
   }

   bool ProcessMessage(std::vector<std::string>& shots) {
      return ConfSlave::ProcessMessage(shots);
   }

   bool MessageHasPayload(const std::vector<std::string>& shots) {
      return ConfSlave::MessageHasPayload(shots);
   }
   bool MessageRequiresNoFurtherAction(const protoMsg::ConfType_Type& type) {
      return ConfSlave::MessageRequiresNoFurtherAction(type);
   }
   
   bool ProcessRestartMsg(const protoMsg::ConfType& configTypeMessage,
           const std::vector<std::string>& shots) {
      LOG(DEBUG) << "MockProcessRestartMsg";
      return ConfSlave::ProcessRestartMsg(configTypeMessage, shots);
   }

   bool ProcessShutdownMsg(const protoMsg::ConfType& configTypeMessage,
           const std::vector<std::string>& shots) {
      LOG(DEBUG) << "ProcessShutdownMsg";
      mShutDownConfReceived = ConfSlave::ProcessShutdownMsg(configTypeMessage, shots);

      if (mShutDownConfReceived) {
         protoMsg::ShutdownMsg shutdownMsg;
         shutdownMsg.ParseFromString(shots[1]);
         mShutDownConfValue = shutdownMsg.now();
         LOG(INFO) << "MockShutdown command received, with status: " << shutdownMsg.now();
      }

      return mShutDownConfReceived;
   }

  

   bool mShutDownConfReceived;
   bool mShutDownConfValue;
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


