/* 
 * Author: Robert Weber
 *
 * Created on October 30, 2012, 8:18 AM
 */
#pragma once
#include "ConfMaster.h"
#include "MockConf.h"
#include <memory>

class MockConfMaster : public ConfMaster {
public:

   MockConfMaster() : ConfMaster(), mConfLocation("resources/test.yaml"), mConf(mConfLocation),
   mOverrideInternalRepair(false),
   mInternalRepair(true),
   mValidateEthFailCount(0),
   mIgnoreConfValidate(true),
   mValidConfValidation(true) {
      mEthInfo.Initialize();
   }

   void RepairConf(Conf& conf) {
   }

   Conf GetConf(void) {
      MockConf conf(mConfLocation);
      return std::move(conf);
   }

   bool InternallyRepairBaseConf(Conf& conf, EthInfo& ethInfo) LR_OVERRIDE {
      if (mOverrideInternalRepair) {
         return mInternalRepair;
      }
      return ConfMaster::InternallyRepairBaseConf(conf, ethInfo);
   }

   bool InternallyRepairBaseConf(Conf& conf) {
      return ConfMaster::InternallyRepairBaseConf(conf);
   }

   void RepairEthConfFieldsWithDefaults(Conf& conf, EthInfo& ethInfo) LR_OVERRIDE {
      if (mValidateEthFailCount > 0) {
         mValidateEthFailCount--;
      }
      return ConfMaster::RepairEthConfFieldsWithDefaults(conf, ethInfo);
   }

   bool ValidateEthConfFields(Conf& conf, EthInfo& ethInfo) LR_OVERRIDE {
      if (mValidateEthFailCount > 0) {
         return false;
      } else if (mValidateEthFailCount == 0) {
         return true;
      }
      mValidateEthFailCount = -1;
      return ConfMaster::ValidateEthConfFields(conf, ethInfo);
   }

   bool ValidateConfFieldValues(Conf& conf, ::google::protobuf::Message& msg, const protoMsg::ConfType_Type &type) LR_OVERRIDE {
      if (mIgnoreConfValidate) {
         return true;
      }
      mValidConfValidation = ConfMaster::ValidateConfFieldValues(conf, msg, type);
      return mValidConfValidation;
   }

   std::vector<std::string> ParseRequest(const std::vector<std::string>& msgs, bool& gotNewConfig, bool& sendRestarts) {
      return ConfMaster::ParseRequest(msgs, gotNewConfig, sendRestarts);
   }

   bool ProcessBaseConfigRequest(Conf& conf, const std::string& msg) {
      return ConfMaster::ProcessBaseConfigRequest(conf, msg);
   }

   bool ProcessQosmosConfigRequest(Conf& conf, const std::string& msg) {
      return ConfMaster::ProcessQosmosConfigRequest(conf, msg);
   }

   bool ProcessSyslogConfigRequest(Conf& conf, const std::string& msg) {
      return ConfMaster::ProcessSyslogConfigRequest(conf, msg);
   }

   bool ProcessInterfaceConfigRequest(AbstractConf& conf, const std::string& msg) {
      return ConfMaster::ProcessInterfaceConfigRequest(conf, msg);
   }

   bool ProcessNtpRequest(AbstractConf& conf, const std::string& msg) {
      return ConfMaster::ProcessNtpRequest(conf, msg);
   }

   bool IsRestartRequest(const protoMsg::ConfType& configTypeMessage) {
      return ConfMaster::IsRestartRequest(configTypeMessage);
   }

   bool IsShutdownRequest(const protoMsg::ConfType& configTypeMessage) {
      return ConfMaster::IsShutdownRequest(configTypeMessage);
   }

   void UpdateCachedMessages(Conf& conf) {
      return ConfMaster::UpdateCachedMessages(conf);
   }

   void UpdateCachedMessage(AbstractConf& conf) {
      return ConfMaster::UpdateCachedMessage(conf);
   }

   std::string SerializeCachedConfig(const protoMsg::ConfType& configTypeMessage) {
      return ConfMaster::SerializeCachedConfig(configTypeMessage);
   }

   bool ReconcileNewConf(const protoMsg::ConfType& configTypeMessage, Conf& conf,
           const std::string& message) {
      return ConfMaster::ReconcileNewConf(configTypeMessage, conf, message);
   }

   bool ReconcileNewAbstractConf(const protoMsg::ConfType& configTypeMessage, AbstractConf& conf,
           const std::string& message) {
      return ConfMaster::ReconcileNewAbstractConf(configTypeMessage, conf, message);
   }

   void SetConfLocation(const std::string& path) {
      mConfLocation = path;
      mConf.setPath(path);
   }
   std::string mConfLocation;
      bool mOverrideInternalRepair;
   bool mInternalRepair;
   int mValidateEthFailCount;
   bool mIgnoreConfValidate;
   bool mValidConfValidation;
private:

   MockConf mConf;
};



