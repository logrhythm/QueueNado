/* 
 * File:   MockNtpConfigCommand.h
 * Author: kjell
 *
 * Created on August 1, 2013, 2:09 PM
 */

#pragma once
#include "NtpConfigCommand.h"
#include "CommandReply.pb.h"
#include "ProcessManager.h"
#include <string>

struct MockNtpConfigCommand : public NtpConfigCommand {

   MockNtpConfigCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager)
   : NtpConfigCommand(request, processManager) {
   }

   ~MockNtpConfigCommand() {
   }

   protoMsg::ProcessReply IsServerAlive(const std::string& server) LR_OVERRIDE {
      if (oneServerAlive.empty()) {
         return NtpConfigCommand::IsServerAlive(server);
      }
      protoMsg::ProcessReply reply = NtpConfigCommand::IsServerAlive(server);
      if (oneServerAlive == server) {
         reply.set_success(true);
         reply.set_returncode(0);
      } else {
         reply.set_success(false);
         reply.set_returncode(-1);
      }

      return reply;
   }


   protoMsg::ProcessReply ForceTimeSync(const std::string & server) LR_OVERRIDE {
      if (oneServerAlive.empty()) {
         return NtpConfigCommand::ForceTimeSync(server);
      }
      
      protoMsg::ProcessReply reply = NtpConfigCommand::ForceTimeSync(server);
      if (oneServerAlive == server) {
         reply.set_success(true);
         reply.set_returncode(0);
      } else {
         reply.set_success(false);
         reply.set_returncode(-1);
      }

      return reply;
   }


   std::string oneServerAlive;
};
