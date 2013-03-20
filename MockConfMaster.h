/* 
 * Author: Robert Weber
 *
 * Created on October 30, 2012, 8:18 AM
 */
#pragma once
#include "ConfMaster.h"

namespace networkMonitor {
class MockConfMaster : public ConfMaster {
public:
   MockConfMaster() {
      
   }
   Conf GetConf(void) {
      return Conf("resources/test.yaml");
   }
   std::vector<std::string> ParseRequest(const std::vector<std::string>& msgs, bool& gotNewConfig, bool& sendRestarts) {
      return ConfMaster::ParseRequest(msgs,gotNewConfig,sendRestarts);
   }
};
}

