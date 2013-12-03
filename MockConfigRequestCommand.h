
#pragma once
#include "ConfigRequestCommand.h"
#include "include/global.h"
#include "gmock/gmock.h"
#include "BaseConfMsg.pb.h"
#include "ConfTypeMsg.pb.h"
#include "CommandReply.pb.h"
#include "CommandRequest.pb.h"
#include "ConfigDefaultsRequest.pb.h"
#include "ConfigDefaults.pb.h"

#include <map>
#include <thread>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>

struct MockConfigRequestCommand : public ConfigRequestCommand {      // name, default, max, min

   struct Ranges {
      std::string Min;
      std::string Max;
   };

   //tuple< paramName, default value, valid range>
   typedef std::tuple<std::string, std::string, Ranges> OneConfDefault;
   typedef std::vector<OneConfDefault> ManyConfDefaults;

   MockConfigRequestCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager)
   : ConfigRequestCommand(request, processManager), mMockExecuteRequest(false) {
   }

   ManyConfDefaults HelperGetConfDefaults(protoMsg::ConfType::Type type) {
      auto it = mMockConfValues.find(type);
      if (mMockConfValues.end() != it) {
         return it->second;
      }

      ManyConfDefaults empty;
      return empty;
   }

   bool HelperLoadConfDefault(OneConfDefault& loadMe, const std::string key, const ManyConfDefaults& values) {
      auto found = std::find_if(values.begin(), values.end(),
              [&](const OneConfDefault & current) {
                 return key == std::get<0>(current); });
      if (values.end() != found) { //If true, value was key in vector
         loadMe = *found;
         return true;
      }
      return false;
   }

   // Somewhat messy. We mock and exercise that we actually can get conf default values
   // It is basically just an exercise in moving values between protoMessages 
   // ConfigDefaultsRequest, ConfigDefaults, CommandReply and CommandRequest

   protoMsg::CommandReply ExecuteRequest(const protoMsg::ConfigDefaultsRequest& request) override {
      if (!mMockExecuteRequest) {
         return ConfigRequestCommand::ExecuteRequest(request);
      }

      protoMsg::CommandReply reply;
      bool success = false;
      // In our MAP we keep the mock values.  Loop through the request and see if we have a map matching
      // if we do then return that field. If not then it is considered a "failure"
      protoMsg::ConfigDefaults protoDefaults;
      const int size = request.requestedconfigparams_size();
      if (request.has_type() && size > 0) { //E.g BASE or SYSLOG
         
         //Vector of maps<param name, default val, valid range>
         auto allDefaultsForType = HelperGetConfDefaults(request.type());
         
         for (int index = 0; index < size; ++index) {
            std::string paramName = request.requestedconfigparams(index);
            OneConfDefault setting;
            
            //If specified param default value exists in vector: then add that default to values
            if (HelperLoadConfDefault(setting, paramName, allDefaultsForType)) {
               protoMsg::ConfigDefaults_DefaultValues values;        //Repeated (array of DefaultValues)
               values.set_type(request.type());                      //E.g. BASE or SYSLOG
               values.set_configname(std::get<0>(setting));          //paramName
               values.set_defaultu64(std::get<1>(setting));          //default value
               values.set_maxu64(std::get<2>(setting).Max);          //range min
               values.set_minu64(std::get<2>(setting).Min);          //range max

               auto valuePtr = protoDefaults.add_values();
               *valuePtr = values;
               success = true;
            }
         }
      }

      reply.set_success(success);
      reply.set_result(protoDefaults.SerializeAsString());
      return reply;
   }

   void EnabledMockExecuteRequest(protoMsg::ConfType::Type type, const ManyConfDefaults& values) {
      mMockExecuteRequest = true;
      mMockConfValues[type] = values;
   }

   virtual ~MockConfigRequestCommand() {
   }
private:


   std::map<protoMsg::ConfType::Type, ManyConfDefaults> mMockConfValues;
   bool mMockExecuteRequest;
};

class GMockConfigRequestCommand : public MockConfigRequestCommand {
public:

   GMockConfigRequestCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager) :
   MockConfigRequestCommand(request, processManager) {
   }

   virtual ~GMockConfigRequestCommand() = default;
};
