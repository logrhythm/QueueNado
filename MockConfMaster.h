/* 
 * Author: Robert Weber
 *
 * Created on October 30, 2012, 8:18 AM
 */
#pragma once
#include "ConfMaster.h"
#include <memory>

namespace networkMonitor {

   class MockConfMaster : public ConfMaster {
   public:

      MockConfMaster() : ConfMaster(), mConfLocation("resources/test.yaml"), mConf(mConfLocation) {

      }

      Conf GetConf(void) {
         Conf conf(mConfLocation);
         return std::move(conf);
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

      bool IsRestartRequest(const protoMsg::ConfType& configTypeMessage) {
         return ConfMaster::IsRestartRequest(configTypeMessage);
      }

      void UpdateCachedMessages(Conf& conf) {
         return ConfMaster::UpdateCachedMessages(conf);
      }

      std::string SerializeCachedConfig(const protoMsg::ConfType& configTypeMessage) {
         return ConfMaster::SerializeCachedConfig(configTypeMessage);
      }

      bool ReconcileNewConf(const protoMsg::ConfType& configTypeMessage, Conf& conf,
              const std::string& message) {
         return ConfMaster::ReconcileNewConf(configTypeMessage, conf, message);
      }

      void SetConfLocation(const std::string& path) {
         mConfLocation = path;
         mConf.setPath(path);
      }
      std::string mConfLocation;
   private:

      Conf mConf;
   };

}

